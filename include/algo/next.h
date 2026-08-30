#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

namespace wz::core::algo::next
{
    enum class execution_status
    {
        completed,
        truncated,
    };

    [[nodiscard]] constexpr bool was_truncated(execution_status status) noexcept
    {
        return status == execution_status::truncated;
    }

    template<typename R>
    concept ReadableRange = std::ranges::input_range<const R>;

    template<typename Out, typename InVal, typename F>
    concept CanTransformInto =
        requires(Out& out, F& fn, InVal value)
    {
        { std::invoke(fn, value) };
        { out.push(std::invoke(fn, value)) } -> std::convertible_to<bool>;
    };

    namespace detail
    {
        template<ReadableRange In, typename Step>
        execution_status consume_until_rejected(const In& in, Step&& step)
        {
            const auto rejected = std::ranges::find_if(
                in,
                [&step](auto&& value)
                {
                    return !static_cast<bool>(std::invoke(step, value));
                });

            return rejected == std::ranges::end(in)
                ? execution_status::completed
                : execution_status::truncated;
        }
    }

    template<ReadableRange In, typename Out, typename F>
        requires CanTransformInto<
            Out,
            std::ranges::range_reference_t<const In>,
            F
        >
    execution_status transform(const In& in, Out& out, F&& fn)
    {
        return detail::consume_until_rejected(
            in,
            [&out, &fn](auto&& value)
            {
                return out.push(std::invoke(fn, value));
            });
    }

    template<ReadableRange In, typename Out, typename Pred>
        requires requires(
            Out& out,
            std::ranges::range_reference_t<const In> value,
            Pred& pred)
    {
        { out.push(value) } -> std::convertible_to<bool>;
        { std::invoke(pred, value) } -> std::convertible_to<bool>;
    }
    execution_status filter(const In& in, Out& out, Pred&& pred)
    {
        return detail::consume_until_rejected(
            in,
            [&out, &pred](auto&& value)
            {
                return !std::invoke(pred, value) || out.push(value);
            });
    }

    template<ReadableRange In, typename T, typename F>
        requires requires(
            T accumulator,
            std::ranges::range_reference_t<const In> value,
            F& fn)
    {
        { std::invoke(fn, accumulator, value) } -> std::convertible_to<T>;
    }
    T reduce(const In& in, T init, F&& fn)
    {
        std::ranges::for_each(
            in,
            [&init, &fn](auto&& value)
            {
                init = std::invoke(fn, init, value);
            });
        return init;
    }

    template<typename T>
    concept PipelineOperation =
        requires
    {
        typename std::remove_cvref_t<T>::pipeline_operation_tag;
    };

    template<typename... Ops>
    struct pipeline_t
    {
        std::tuple<Ops...> ops;

        template<ReadableRange In, typename Out>
        execution_status operator()(const In& in, Out& out) const
        {
            return detail::consume_until_rejected(
                in,
                [this, &out](auto&& value)
                {
                    return apply_all(value, out);
                });
        }

        template<typename Value, typename Out>
        bool apply_all(Value&& value, Out& out) const
        {
            return apply_all_impl<0>(std::forward<Value>(value), out);
        }

        template<std::size_t Index, typename Value, typename Out>
        bool apply_all_impl(Value&& value, Out& out) const
        {
            if constexpr (Index == sizeof...(Ops))
            {
                return out.push(std::forward<Value>(value));
            }
            else
            {
                const auto& operation = std::get<Index>(ops);
                return operation.template apply<Index>(
                    std::forward<Value>(value),
                    out,
                    *this);
            }
        }
    };

    template<typename F>
    struct map_t
    {
        using pipeline_operation_tag = void;

        F fn;

        template<ReadableRange In, typename Out>
        execution_status operator()(const In& in, Out& out) const
        {
            return transform(in, out, fn);
        }

        template<std::size_t Index, typename Value, typename Out, typename Pipeline>
        bool apply(Value&& value, Out& out, const Pipeline& pipeline) const
        {
            auto transformed = std::invoke(fn, std::forward<Value>(value));
            return pipeline.template apply_all_impl<Index + 1>(
                std::move(transformed),
                out);
        }
    };

    template<typename P>
    struct filter_t
    {
        using pipeline_operation_tag = void;

        P pred;

        template<ReadableRange In, typename Out>
        execution_status operator()(const In& in, Out& out) const
        {
            return next::filter(in, out, pred);
        }

        template<std::size_t Index, typename Value, typename Out, typename Pipeline>
        bool apply(Value&& value, Out& out, const Pipeline& pipeline) const
        {
            if (std::invoke(pred, value))
            {
                return pipeline.template apply_all_impl<Index + 1>(
                    std::forward<Value>(value),
                    out);
            }
            return true;
        }
    };

    template<typename F, typename P>
    struct map_filter_t
    {
        using pipeline_operation_tag = void;

        F fn;
        P pred;

        template<ReadableRange In, typename Out>
        execution_status operator()(const In& in, Out& out) const
        {
            return detail::consume_until_rejected(
                in,
                [this, &out](auto&& value)
                {
                    auto transformed = std::invoke(fn, value);
                    return !std::invoke(pred, transformed)
                        || out.push(std::move(transformed));
                });
        }

        template<std::size_t Index, typename Value, typename Out, typename Pipeline>
        bool apply(Value&& value, Out& out, const Pipeline& pipeline) const
        {
            auto transformed = std::invoke(fn, std::forward<Value>(value));
            if (std::invoke(pred, transformed))
            {
                return pipeline.template apply_all_impl<Index + 1>(
                    std::move(transformed),
                    out);
            }
            return true;
        }
    };

    template<typename... Left, typename... Right>
    auto operator|(pipeline_t<Left...> left, pipeline_t<Right...> right)
    {
        return pipeline_t<Left..., Right...>{
            std::tuple_cat(std::move(left.ops), std::move(right.ops))
        };
    }

    template<typename... Left, PipelineOperation Right>
    auto operator|(pipeline_t<Left...> left, Right right)
    {
        return pipeline_t<Left..., std::decay_t<Right>>{
            std::tuple_cat(
                std::move(left.ops),
                std::tuple<std::decay_t<Right>>{std::move(right)})
        };
    }

    template<PipelineOperation Left, typename... Right>
    auto operator|(Left left, pipeline_t<Right...> right)
    {
        return pipeline_t<std::decay_t<Left>, Right...>{
            std::tuple_cat(
                std::tuple<std::decay_t<Left>>{std::move(left)},
                std::move(right.ops))
        };
    }

    template<typename F, typename P>
    auto operator|(map_t<F> map_operation, filter_t<P> filter_operation)
    {
        return map_filter_t<F, P>{
            std::move(map_operation.fn),
            std::move(filter_operation.pred)
        };
    }

    template<PipelineOperation Left, PipelineOperation Right>
    auto operator|(Left left, Right right)
    {
        return pipeline_t<std::decay_t<Left>, std::decay_t<Right>>{
            std::tuple<std::decay_t<Left>, std::decay_t<Right>>{
                std::move(left),
                std::move(right)
            }
        };
    }

    template<typename F>
    map_t<std::decay_t<F>> map(F&& fn)
    {
        return {std::forward<F>(fn)};
    }

    template<typename P>
    filter_t<std::decay_t<P>> filter(P&& pred)
    {
        return {std::forward<P>(pred)};
    }
}
