#include <gtest/gtest.h>

#include <algo/algo.h>
#include <algo/ops.h>
#include <containers/buffer.h>

TEST(AlgoBaseline, MultipleApplyOperationsShareOriginalInputAndBoundedOutput)
{
    int input_storage[3] = { 1, 2, 3 };
    int output_storage[3] = { 0, 0, 0 };

    auto input = wz::core::containers::Buffer<int>::wrap_existing(input_storage, 3, 3);
    auto output = wz::core::containers::Buffer<int>::wrap(output_storage, 3);

    wz::core::algo::apply(
        input,
        output,
        wz::core::algo::ops::map([](int value) { return value + 1; }),
        wz::core::algo::ops::map([](int value) { return value * 2; }));

    EXPECT_EQ(output.count(), 3u);
    EXPECT_EQ(output.overflow_count(), 3u);
    EXPECT_EQ(output_storage[0], 2);
    EXPECT_EQ(output_storage[1], 3);
    EXPECT_EQ(output_storage[2], 4);
}
