#include <SoftHeapCpp.h>

#include <gtest/gtest.h>

TEST(SoftHeapCpp, AsSimpleHeap)
{
    SoftHeapCpp<int> heap(10000);

    for (int i = 0; i < 10; ++i)
        heap.Insert(i);

    //for (int i = 0; i < 10; ++i)
    //{
    //    EXPECT_EQ(i, heap.DeleteMin());
    //}
}
