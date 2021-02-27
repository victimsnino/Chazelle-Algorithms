#include "Common.h"
#include "SoftHeapC.h"

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

    ComparableObject& operator=(ComparableObject&& other) noexcept
    {
        if (this == &other)
            return *this;
        m_v = other.m_v;
        return *this;
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

    friend std::ostream& operator<<(std::ostream& out, const ComparableObject& obj){return out << obj.m_v;}

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

TEST(SoftHeapCpp, ComapareWithSoftHeapC)
{
    auto             r = Utils::CalculateRByEps(1.0 / 3.0);

    for (auto count : { 3, 5, 10, 30, 40, 51, 73, 91, 132 })
    {
        SoftHeapCpp<int> heap_cpp{ r };
        SoftHeapC        heap_c{ static_cast<int>(r) };

        for (int i = 0; i < count; ++i)
        {
            heap_c.Insert(i);
            heap_cpp.Insert(i);
        }

        for (int i = 0; i < count / 2; ++i)
        {
            EXPECT_EQ(heap_c.DeleteMin(), heap_cpp.DeleteMin()) << count << " " << i;
        }

        for (int i = 1; i < count/2; ++i)
        {
            heap_c.Insert(count*i);
            heap_cpp.Insert(count*i);
        }

        for (int i = 1; i < count / 2; ++i)
        {
            EXPECT_EQ(heap_c.DeleteMin(), heap_cpp.DeleteMin()) << count << " " << i;
        }
    }
}

TEST(SoftHeapCpp, CompareWithSoftHeapCShuffle)
{
    auto             r = Utils::CalculateRByEps(1.0 / 3.0);

    for (auto count : {5, 10})
    {
        SoftHeapCpp<int> heap_cpp{ r };
        SoftHeapC        heap_c{ static_cast<int>(r) };

        std::vector<int> values;
        values.resize(count);
        std::iota(values.begin(), values.end(), 0);

        std::random_device rd;
        std::mt19937       g(rd());
        std::ranges::shuffle(values, g);


        for (auto val : values)
        {
            heap_c.Insert(val);
            heap_cpp.Insert(val);
        }

        for (int i = 0; i < count; ++i)
            std::cout << heap_c.DeleteMin() << ", ";

        std::cout << std::endl;
        for (int i = 0; i < count; ++i)
            std::cout << heap_cpp.DeleteMin() << ", ";
        std::cout << "=======" << std::endl;
        
        /*for (int i = 0; i < count; ++i)
        {
            EXPECT_EQ(heap_c.DeleteMin(), heap_cpp.DeleteMin()) << count << " " << i;
        }*/
    }
}