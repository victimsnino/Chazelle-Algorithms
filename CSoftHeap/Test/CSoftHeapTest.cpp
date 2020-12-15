#include <gtest/gtest.h>

#include <CSoftHeap.h>

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
    CSoftHeap heap(0);
    int count = 9;

    for (int i = 1; i < count; ++i)
        heap.insert(i);

    // 0 -> 1 && 2 = 2,1 -> 3 && 4 = 4,3

    for (int i = 1; i < count; ++i)
    {
        auto value = heap.deletemin();
        std::cout << i << ":" << value << std::endl;
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
