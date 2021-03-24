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
#include "MSTStackSubgraph.h"

#include <Graph.h>

#include <vector>

namespace MST::Details
{
class MSTStack
{
public:
    MSTStack(Graph::Graph& graph, size_t c);

    void Push(size_t vertex);

    // Contact last node, move vertex to prev-last node, meld heaps all heaps except of H(K) and H(k-1, k).
    // Extract data from these heaps. Pop min links for each prev. subgraphs
    MSTSoftHeapDecorator::ExtractedItems pop();

    ISubGraph& top()
    {
        assert(!m_nodes.empty());
        return m_nodes.back();
    }

    size_t size() const { return m_nodes.empty() ? 0 : m_nodes.back().GetIndex() + 1; }

    auto view()
    {
        return std::ranges::views::transform(m_nodes, [](SubGraph& sub_graph)-> ISubGraph& { return sub_graph; });
    }

private:
    void PushNode(size_t vertex);

    void AddNewBorderEdgesAfterPush();
    void DeleteOldBorderEdgesAndUpdateMinLinksAfterPush();

    size_t GetMaxHeight() const { return m_sizes_per_height.size() - 1; }
    size_t IndexToHeight(size_t index) const { return GetMaxHeight() - index; }
private:
    Graph::Graph&       m_graph;
    std::list<SubGraph> m_nodes{};
    std::list<size_t>   m_vertices_inside{};

    const size_t              m_r;
    const std::vector<size_t> m_sizes_per_height;
};
}
