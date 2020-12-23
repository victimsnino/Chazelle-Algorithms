#include "Common.h"

#include <numeric>
#include <random>
#include <SoftHeapCpp.h>

#include <gtest/gtest.h>

struct ComparableObject
{
    ComparableObject(int v)
        : m_v(v)
    {
        if (s_debug_logs)
            std::cout << "Create object " << v << std::endl;
    }

    ~ComparableObject()
    {
        if (s_debug_logs)
            std::cout << "Delete object " << m_v << std::endl;
    }

    ComparableObject(ComparableObject&& other) noexcept
        : m_v(other.m_v)
    {
        other.m_v = -1;
        if (s_debug_logs)
            std::cout << "Move constructor " << m_v << std::endl;
    }

    ComparableObject(const ComparableObject& other) noexcept
        : m_v(other.m_v)
    {
        if (s_debug_logs)
            std::cout << "Copy constructor " << m_v << std::endl;
    }

    friend bool operator<(const ComparableObject& lhs, const ComparableObject& rhs) { return lhs.m_v < rhs.m_v; }
    friend bool operator<=(const ComparableObject& lhs, const ComparableObject& rhs) { return !(rhs < lhs); }
    friend bool operator>(const ComparableObject& lhs, const ComparableObject& rhs) { return rhs < lhs; }
    friend bool operator>=(const ComparableObject& lhs, const ComparableObject& rhs) { return !(lhs < rhs); }
    friend bool operator==(const ComparableObject& lhs, const ComparableObject& rhs) { return lhs.m_v == rhs.m_v; }
    friend bool operator!=(const ComparableObject& lhs, const ComparableObject& rhs) { return !(lhs == rhs); }

    static bool s_debug_logs;
private:
    int m_v;
};

bool ComparableObject::s_debug_logs = false;

TEST(SoftHeapCpp, AsSimpleHeap)
{
    constexpr size_t              count = 30;
    SoftHeapCpp<ComparableObject> heap(10000);

    for (size_t i = 0; i < count; ++i)
        heap.Insert(i);

    for (size_t i = 0; i < count; ++i)
    {
        EXPECT_EQ(i, heap.DeleteMin());
    }
}

TEST(SoftHeapCpp, WithSomeR)
{
    SoftHeapCpp<int>    heap(0);
    constexpr const int count = 9;

    for (int i = 1; i < count; ++i)
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
            EXPECT_EQ(i + 1, value);
        else
            EXPECT_EQ(i - 1, value);
    }
}

TEST(SoftHeapCpp, AsKthLargestElement)
{
    for (auto count : { 3, 5, 10, 30, 40, 51, 73, 91, 132 })
    {
        std::vector<int> values;
        values.resize(count);
        std::iota(values.begin(), values.end(), 0);

        std::random_device rd;
        std::mt19937       g(rd());
        std::shuffle(values.begin(), values.end(), g);


        for (int i = 0; i < count; ++i)
            EXPECT_EQ(Utils::SoftHeapSelect<SoftHeapCpp<int>>(values, i), i);
    }
}