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

#include "MSTStack.h"

#include "MSTUtils.h"

#include <Common.h>

static std::vector<size_t> InitTargetSizesPerHeight(const Graph::Graph& graph, size_t c)
{
    const auto max_height = MST::FindMaxHeight(graph, c);
    const auto t          = MST::FindParamT(graph, max_height) + 1; // TODO: Remove +1

    std::vector<size_t> out{};

    // Leafs
    out.push_back(1);

    for (uint32_t i = 1; i <= max_height; ++i)
        out.push_back(MST::CalculateTargetSize(t, i));
    return out;
}

namespace MST::Details
{
MSTStack::MSTStack(Graph::Graph& graph, size_t c)
    : m_graph{graph}
    , m_r{Utils::CalculateRByEps(1 / static_cast<double>(c))}
    , m_sizes_per_height{InitTargetSizesPerHeight(graph, c)}
{
    PushNode(graph.FindRootOfSubGraph(0));
}

SoftHeapCpp<EdgePtrWrapper>::ExtractedItems MSTStack::Pop()
{
    auto& last_subgraph = m_nodes.back();
    last_subgraph.Contract(m_graph, m_vertices_inside);

    if (m_nodes.size() == 1)
    {
        const size_t index = last_subgraph.GetIndex();
        m_nodes.emplace_front(m_vertices_inside.cbegin(),
                              index - 1,
                              m_sizes_per_height[GetMaxHeight() - index - 1],
                              m_r);
    }

    std::next(m_nodes.rbegin())->Meld(last_subgraph);
    auto data = last_subgraph.ExtractItems();

    m_nodes.pop_back();

    std::for_each_n(m_nodes.begin(),
                    m_nodes.size() - 1,
                    [](SubGraph& graph){graph.PopMinLink();});

    m_nodes.back().PopMinLink(true);

    return data;
}

void MSTStack::PushNode(size_t vertex)
{
    auto index = size();
    // Empty stack, initialization stage
    if (index == 0)
        index = GetMaxHeight();

    m_vertices_inside.push_back(m_graph.FindRootOfSubGraph(vertex));
    m_nodes.emplace_back(m_vertices_inside.cbegin(),
                         index,
                         m_sizes_per_height[GetMaxHeight() - index],
                         m_r);

    AddNewBorderEdgesAfterPush();
    DeleteOldBorderEdgesAndUpdateMinLinksAfterPush();
}

void MSTStack::AddNewBorderEdgesAfterPush()
{
    auto& new_node = m_nodes.back();

    m_graph.ForEachAvailableEdge([&](const Graph::Details::Edge& edge)
    {
        const auto vertices = edge.GetCurrentSubgraphs(m_graph);

        const auto outside_vertices = vertices |
                rgv::filter(Utils::IsInRange(m_vertices_inside)) |
                Utils::to_vector;

        if (outside_vertices.empty() || outside_vertices.size() == 2)
            return;

        assert(new_node.GetVertices().size() == 1 &&
               (vertices[0] == *new_node.GetVertices().begin() ||
                   vertices[1] == *new_node.GetVertices().begin()));

        new_node.PushToHeap(EdgePtrWrapper{edge});
    });
}

void MSTStack::DeleteOldBorderEdgesAndUpdateMinLinksAfterPush()
{
    auto& new_node = m_nodes.back();
    assert(new_node.GetVertices().size() == 1);

    auto condition = [&](const EdgePtrWrapper& edge)
    {
        return Utils::IsRangeContains(edge->GetCurrentSubgraphs(m_graph), *new_node.GetVertices().begin());
    };

    std::for_each_n(m_nodes.begin(),
                    m_nodes.size() - 1,
                    [&](SubGraph& node)
                    {
                        const auto old_border_edges = node.DeleteAndReturnIf(condition);
                        if (old_border_edges.empty())
                            return;

                        auto min = std::min_element(old_border_edges.cbegin(), old_border_edges.cend(),
                                                         [](const EdgePtrWrapper& left,const EdgePtrWrapper& right)
                                                         {
                                                             return left.GetWorkingCost() < right.GetWorkingCost();
                                                         });
                        node.AddToMinLinks(*min);
                    });
}
} // namespace MST::Details
