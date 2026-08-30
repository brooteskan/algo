#include <algo/next.h>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <utility>

namespace
{
    template<typename T, std::size_t Capacity>
    struct bounded_sink
    {
        std::array<T, Capacity> values{};
        std::size_t count{};

        bool push(T value)
        {
            if (count == Capacity)
            {
                return false;
            }
            values[count++] = std::move(value);
            return true;
        }
    };
}

using wz::core::algo::next::execution_status;
using wz::core::algo::next::filter;
using wz::core::algo::next::map;
using wz::core::algo::next::was_truncated;

TEST(AlgoNextStatus, TransformReportsCompleted)
{
    constexpr std::array input{1, 2, 3};
    bounded_sink<int, 3> output;

    const auto status = wz::core::algo::next::transform(
        input,
        output,
        [](int value) { return value * 2; });

    EXPECT_EQ(status, execution_status::completed);
    EXPECT_FALSE(was_truncated(status));
    EXPECT_EQ(output.count, 3);
    EXPECT_EQ(output.values, (std::array{2, 4, 6}));
}

TEST(AlgoNextStatus, TransformReportsTruncated)
{
    constexpr std::array input{1, 2, 3};
    bounded_sink<int, 2> output;

    const auto status = wz::core::algo::next::transform(
        input,
        output,
        [](int value) { return value * 2; });

    EXPECT_EQ(status, execution_status::truncated);
    EXPECT_TRUE(was_truncated(status));
    EXPECT_EQ(output.count, 2);
    EXPECT_EQ(output.values, (std::array{2, 4}));
}

TEST(AlgoNextStatus, TransformStopsAtFirstRejectedValue)
{
    constexpr std::array input{1, 2, 3, 4};
    bounded_sink<int, 1> output;
    int evaluations{};

    const auto status = wz::core::algo::next::transform(
        input,
        output,
        [&evaluations](int value)
        {
            ++evaluations;
            return value;
        });

    EXPECT_EQ(status, execution_status::truncated);
    EXPECT_EQ(evaluations, 2);
}

TEST(AlgoNextStatus, ExactCapacityIsCompleted)
{
    constexpr std::array input{1, 2};
    bounded_sink<int, 2> output;

    const auto status = wz::core::algo::next::transform(
        input,
        output,
        [](int value) { return value; });

    EXPECT_EQ(status, execution_status::completed);
    EXPECT_EQ(output.count, 2);
}

TEST(AlgoNextStatus, EmptyInputIsCompleted)
{
    constexpr std::array<int, 0> input{};
    bounded_sink<int, 0> output;

    const auto status = wz::core::algo::next::transform(
        input,
        output,
        [](int value) { return value; });

    EXPECT_EQ(status, execution_status::completed);
}

TEST(AlgoNextStatus, FilterOnlyTruncatesWhenAnAcceptedValueIsRejected)
{
    constexpr std::array input{1, 2, 3, 4, 5, 6};
    bounded_sink<int, 2> limited_output;
    bounded_sink<int, 0> all_filtered_output;

    const auto limited_status = wz::core::algo::next::filter(
        input,
        limited_output,
        [](int value) { return value % 2 == 0; });
    const auto all_filtered_status = wz::core::algo::next::filter(
        input,
        all_filtered_output,
        [](int) { return false; });

    EXPECT_EQ(limited_status, execution_status::truncated);
    EXPECT_EQ(limited_output.count, 2);
    EXPECT_EQ(limited_output.values, (std::array{2, 4}));
    EXPECT_EQ(all_filtered_status, execution_status::completed);
}

TEST(AlgoNextStatus, FilterThenMapComposesWithoutIntermediateStorage)
{
    constexpr std::array input{1, 2, 3, 4};
    bounded_sink<int, 2> output;
    const auto operation =
        filter([](int value) { return value % 2 == 0; })
        | map([](int value) { return value * 10; });

    const auto status = operation(input, output);

    EXPECT_EQ(status, execution_status::completed);
    EXPECT_EQ(output.values, (std::array{20, 40}));
}

TEST(AlgoNextStatus, LongerPipelineReportsTruncation)
{
    constexpr std::array input{1, 2, 3};
    bounded_sink<int, 1> output;
    const auto operation =
        map([](int value) { return value + 1; })
        | map([](int value) { return value * 2; })
        | filter([](int value) { return value > 4; })
        | map([](int value) { return value - 1; });

    const auto status = operation(input, output);

    EXPECT_EQ(status, execution_status::truncated);
    EXPECT_EQ(output.count, 1);
    EXPECT_EQ(output.values[0], 5);
}

TEST(AlgoNextStatus, ComposedPipelinesPreserveStageOrder)
{
    constexpr std::array input{1, 2, 3, 4};
    bounded_sink<int, 1> output;
    const auto select_and_double =
        filter([](int value) { return value % 2 == 0; })
        | map([](int value) { return value * 2; });
    const auto select_and_increment =
        filter([](int value) { return value > 4; })
        | map([](int value) { return value + 1; });
    const auto operation = select_and_double | select_and_increment;

    const auto status = operation(input, output);

    EXPECT_EQ(status, execution_status::completed);
    EXPECT_EQ(output.count, 1);
    EXPECT_EQ(output.values[0], 9);
}

TEST(AlgoNextStatus, FusedMapFilterReportsTruncation)
{
    constexpr std::array input{1, 2, 3};
    bounded_sink<int, 1> output;
    const auto operation =
        map([](int value) { return value * 2; })
        | filter([](int value) { return value >= 2; });

    const auto status = operation(input, output);

    EXPECT_EQ(status, execution_status::truncated);
    EXPECT_EQ(output.values[0], 2);
}

TEST(AlgoNextStatus, FusedMapFilterCanContinueIntoAnotherStage)
{
    constexpr std::array input{1, 2, 3};
    bounded_sink<int, 2> output;
    const auto operation =
        map([](int value) { return value * 2; })
        | filter([](int value) { return value > 2; })
        | map([](int value) { return value + 1; });

    const auto status = operation(input, output);

    EXPECT_EQ(status, execution_status::completed);
    EXPECT_EQ(output.values, (std::array{5, 7}));
}

TEST(AlgoNextStatus, ApplyAllRetainsBooleanSinkContract)
{
    bounded_sink<int, 1> output;
    const auto operation =
        filter([](int value) { return value % 2 == 0; })
        | map([](int value) { return value * 10; });

    EXPECT_TRUE(operation.apply_all(1, output));
    EXPECT_TRUE(operation.apply_all(2, output));
    EXPECT_FALSE(operation.apply_all(4, output));
    EXPECT_EQ(output.values[0], 20);
}
