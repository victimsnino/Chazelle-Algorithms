#include "Common.h"

#include <algorithm>
#include <gtest/gtest.h>

#include <SoftHeapC.h>

#include <numeric>
#include <random>

TEST(SoftHeapC, AsSimpleHeap)
{
    SoftHeapC heap(10000);

    for (int i = 0; i < 10; ++i)
        heap.Insert(i);

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i, heap.DeleteMin());
    }
}

TEST(SoftHeapC, WithSomeR)
{
    SoftHeapC           heap(0);
    constexpr const int count = 9;

    for (int i = 0; i < count; ++i)
        heap.Insert(i);

    // 0 -> 1 && 2 = 2,1 -> 3 && 4 = 4,3
    for (int i = 1; i < count; ++i)
    {
        auto value = heap.DeleteMin();
        //std::cout << i << ":" << value << std::endl;
        if (i == 1 || count - i <= 1)
        {
            EXPECT_EQ(i, value);
            continue;
        }

        if (i % 2 == 0)
            EXPECT_EQ(i+1, value);
        else
            EXPECT_EQ(i - 1, value);
    }
}


TEST(SoftHeapC, AsKthLargestElement)
{
    for (auto count : { 30, 40, 51, 73 })
    {
        std::vector<int> values;
        values.resize(count);
        std::iota(values.begin(), values.end(), 0);

        std::random_device rd;
        std::mt19937       g(rd());
        std::shuffle(values.begin(), values.end(), g);


        for (int i = 0; i < count; ++i)
            EXPECT_EQ(Utils::SoftHeapSelect<SoftHeapC>(values, i), i);
    }
}
