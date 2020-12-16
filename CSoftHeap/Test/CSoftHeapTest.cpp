#include <algorithm>
#include <gtest/gtest.h>

#include <CSoftHeap.h>

#include <numeric>
#include <random>

TEST(CSoftHeap, AsSimpleHeap)
{
    CSoftHeap heap(10000);

    for (int i = 0; i < 10; ++i)
        heap.insert(i);

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i, heap.deletemin());
    }
}

TEST(CSoftHeap, WithSomeR)
{
    CSoftHeap           heap(0);
    constexpr const int count = 9;

    for (int i = 1; i < count; ++i)
        heap.insert(i);

    // 0 -> 1 && 2 = 2,1 -> 3 && 4 = 4,3
    for (int i = 1; i < count; ++i)
    {
        auto value = heap.deletemin();
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

int SoftHeapSelect(std::vector<int> a, int k)
{
    if (a.size() <= 3)
    {
        std::sort(a.begin(), a.end());
        return a[k];
    }

    CSoftHeap heap(CSoftHeap::CalculateRByEps(1.0 / 3.0));
    for (auto& v : a)
        heap.insert(v);

    int max_value = -1;
    for (int i = 0; i < int(a.size() / 3); ++i)
    {
        auto removed = heap.deletemin();
        if (removed >= max_value)
            max_value = removed;
    }
    if (max_value == -1)
        throw std::exception{};

    const auto partition_itr   = std::partition(a.begin(), a.end(), [&](int value) { return value < max_value; });
    const auto partition_index = std::distance(a.begin(), partition_itr);

    // Move max_value to the correct position
    std::partition(partition_itr, a.end(), [&](int value) { return value <= max_value; });

    if (partition_index == k)
        return max_value;

    if (k < partition_index)
        return SoftHeapSelect(std::vector<int>{a.begin(), partition_itr}, k);

    return SoftHeapSelect(std::vector<int>{partition_itr + 1, a.end()}, k - (partition_index + 1));
}

TEST(CSoftHeap, AsKthLargestElement)
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
            EXPECT_EQ(SoftHeapSelect(values, i), i);
    }
}
