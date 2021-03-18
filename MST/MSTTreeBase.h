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

#include <Graph.h>

#include <vector>

namespace MST::Details
{
class SubGraph
{
public:
    SubGraph(std::list<size_t>::const_iterator vertex_itr, size_t index, size_t target_size, size_t r)
        : m_index{index}
        , m_target_size{target_size}
        , m_vertices_begin{vertex_itr}
        , m_vertices_end{std::next(m_vertices_begin)}
        , m_heap{r, index} {}

    size_t GetIndex() const
    {
        return m_index;
    }

    MSTSoftHeapDecorator& GetHeap() { return m_heap; }

private:
    const size_t m_index; // aka k
    const size_t m_target_size;

    const std::list<size_t>::const_iterator m_vertices_begin;
    const std::list<size_t>::const_iterator m_vertices_end;

    MSTSoftHeapDecorator m_heap;

    std::optional<EdgePtrWrapper> m_chain_link_to_next{};
    std::vector<EdgePtrWrapper>   m_min_links_to_next_nodes{};
};

class MSTStack
{
public:
    MSTStack(Graph::Graph& graph, size_t c);

    //void Push();
    //Node Pop();

private:
    void PushNode(size_t vertex);

    size_t GetSize() const { return m_nodes.empty() ? 0 : m_nodes.back().GetIndex() + 1; }
    size_t GetMaxHeight() const { return m_sizes_per_height.size() - 1; }

private:
    Graph::Graph&       m_graph;
    std::list<SubGraph> m_nodes{};
    std::list<size_t>   m_vertices_inside{};


    const size_t              m_r;
    const std::vector<size_t> m_sizes_per_height;
};
}
