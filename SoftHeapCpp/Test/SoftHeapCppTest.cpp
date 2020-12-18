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
    int         m_v;
};

bool ComparableObject::s_debug_logs = false;

TEST(SoftHeapCpp, AsSimpleHeap)
{
    constexpr size_t count = 30;
    SoftHeapCpp<ComparableObject> heap(10000);

    for (size_t i = 0; i < count; ++i)
        heap.Insert(i);

    for (size_t i = 0; i < count; ++i)
    {
        EXPECT_EQ(i, heap.DeleteMin());
    }
}
