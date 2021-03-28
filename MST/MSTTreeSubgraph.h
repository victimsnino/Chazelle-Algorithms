// MIT License
// 
// Copyright (c) 2021 Aleksey Loginov
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
#include "MSTSoftHeapDecorator.h"

#include <ranges>

namespace rg = std::ranges;
namespace rgv = std::ranges::views;

namespace MST::Details
{
struct ISubGraph
{
    virtual ~ISubGraph() { }
};

class SubGraph : public ISubGraph
{
public:
    SubGraph(size_t level_in_tree, size_t target_size, size_t r);

    SubGraph(SubGraph&& other)                 = delete;
    SubGraph(const SubGraph& other)            = delete;
    SubGraph& operator=(const SubGraph& other) = delete;
    SubGraph& operator=(SubGraph&& other)      = delete;

private:
    const size_t m_level_in_tree; // aka k
    const size_t m_target_size;

    std::list<std::unique_ptr<SubGraph>> m_childs{};

    std::vector<MSTSoftHeapDecorator> m_heaps; // i < m_index -> H(i, m_index) cross heap, else - H(m_index)
    std::vector<EdgePtrWrapper>       m_min_links_to_next_nodes{};
};
}