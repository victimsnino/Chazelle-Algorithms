#include <SoftHeapCpp.h>

#include <gtest/gtest.h>

TEST(SoftHeapCpp, AsSimpleHeap)
{
    constexpr size_t count = 30;
    SoftHeapCpp<int> heap(10000);

    for (int i = 0; i < count; ++i)
        heap.Insert(i);

    for (int i = 0; i < count; ++i)
    {
        EXPECT_EQ(i, heap.DeleteMin());
    }
}
