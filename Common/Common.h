// MIT License
// 
// Copyright (c) 2020 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

namespace Utils
{
inline int CalculateRByEps(double eps)
{
    return static_cast<int>(2.0 + 2.0 * std::ceil(std::log(1.0 / eps)));
}

template<typename SoftHeap>
int SoftHeapSelect(std::vector<int> a, int k)
{
    if (a.size() <= 3)
    {
        std::sort(a.begin(), a.end());
        return a[k];
    }

    SoftHeap heap(CalculateRByEps(1.0 / 3.0));
    for (auto& v : a)
        heap.Insert(v);

    int max_value = -1;
    for (int i = 0; i < int(a.size() / 3); ++i)
    {
        auto removed = heap.DeleteMin();
        if (removed >= max_value)
            max_value = removed;
    }

    const auto partition_itr   = std::partition(a.begin(), a.end(), [&](int value) { return value < max_value; });
    const auto partition_index = std::distance(a.begin(), partition_itr);

    if (partition_index == k)
        return max_value;

    if (k < partition_index)
        return SoftHeapSelect<SoftHeap>(std::vector<int>{a.begin(), partition_itr}, k);

    // Move max_value to the correct position
    std::partition(partition_itr, a.end(), [&](int value) { return value <= max_value; });

    return SoftHeapSelect<SoftHeap>(std::vector<int>{partition_itr + 1, a.end()}, k - (partition_index + 1));
}
} // namespace Utils
