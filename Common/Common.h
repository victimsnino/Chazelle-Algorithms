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
#include <functional>
#include <ranges>
#include <vector>

namespace Utils
{
inline size_t CalculateRByEps(double eps)
{
    return static_cast<size_t>(2.0 + 2.0 * std::ceil(std::log(1.0 / eps)));
}

template<typename SoftHeap>
int SoftHeapSelect(std::vector<int> a, int k)
{
    if (a.size() <= 3)
    {
        std::ranges::nth_element(a, a.begin() + k);
        return a[k];
    }

    SoftHeap heap(CalculateRByEps(1.0 / 3.0));
    for (auto& v : a)
        heap.Insert(v);

    int max_value = -1;
    for (int i = 0; i < static_cast<int>(a.size() / 3); ++i)
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


template<typename Rng, typename T>
bool IsRangeContains(const Rng& rng, const T& value)
{
    return std::ranges::find(rng, value) != std::cend(rng);
}

template<typename Rng, typename T = typename Rng::value_type>
std::function<bool(const T&)> IsInRange(const Rng& rng)
{
    return [&](const T& val) { return IsRangeContains(rng, val); };
}
template<std::ranges::range R>
constexpr auto ToVector(R&& r)
{
    using elem_t = std::decay_t<std::ranges::range_value_t<R>>;
    return std::vector<elem_t>{r.begin(), r.end()};
}

struct ToVectorFn
{
    template<typename Rng>
    auto operator()(Rng&& rng) const
    {
        return ToVector(std::forward<Rng>(rng));
    }

    template<typename Rng>
    friend auto operator|(Rng&& rng, ToVectorFn const&)
    {
        return ToVector(std::forward<Rng>(rng));
    }
};

constexpr ToVectorFn to_vector{};
} // namespace Utils
