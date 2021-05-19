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
#include "MSTTreeSubgraph.h"

#include <Graph.h>

#include <vector>

namespace MST::Details
{
class MSTTree
{
public:
    MSTTree(Graph::Graph& graph, size_t t, size_t max_height, size_t initial_vertex);

    void push(const EdgePtrWrapper& extension_edge);

    // Contact last node, move vertex to prev-last node, meld heaps all heaps except of H(K) and H(k-1, k).
    // Extract data from these heaps. Pop min links for each prev. subgraphs
    MSTSoftHeapDecorator::ExtractedItems pop();
    MSTSoftHeapDecorator::ExtractedItems fusion(std::list<SubGraphPtr>::iterator itr, const EdgePtrWrapper& fusion_edge);

    ISubGraph& top();
    size_t     size() const;

    auto view()
    {
        return std::ranges::views::transform(m_active_path,
                                             [](SubGraphPtr& sub_graph)-> ISubGraphPtr
                                             {
                                                 return sub_graph;
                                             });
    }

    auto view() const
    {
        return std::ranges::views::transform(m_active_path,
                                             [](const SubGraphPtr& sub_graph)-> const ISubGraphPtr
                                             {
                                                 return sub_graph;
                                             });
    }

    std::list<Graph::Graph> CreateSubGraphs(const std::set<size_t>& bad_edges);
    std::list<size_t>       GetVerticesInside();
    const std::set<size_t>& GetBadEdges() const { return m_bad_edges; }
private:
    void PushNode(size_t vertex);

    void AddNewBorderEdgesAfterPush();
    void DeleteOldBorderEdgesAndUpdateMinLinksAfterPush();

    size_t GetMaxHeight() const { return m_sizes_per_height.size() - 1; }
    size_t IndexToHeight(size_t index) const { return GetMaxHeight() - index; }
private:
    Graph::Graph&          m_graph;
    std::list<SubGraphPtr> m_active_path{};
    std::set<size_t>       m_bad_edges{};

    const size_t              m_r;
    const std::vector<size_t> m_sizes_per_height;
};
}

