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

#include "MSTTreeBase.h"

#include "MSTUtils.h"

#include <Common.h>

#include <ranges>

namespace rg = std::ranges;
namespace rgv = std::ranges::views;

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
std::list<EdgePtrWrapper> SubGraph::DeleteAndReturnIf(std::function<bool(const EdgePtrWrapper& edge)> func)
{
    return m_heap.DeleteAndReturnIf(std::move(func));
}

MSTStack::MSTStack(Graph::Graph& graph, size_t c)
    : m_graph{graph}
    , m_r{Utils::CalculateRByEps(1 / static_cast<double>(c))}
    , m_sizes_per_height{InitTargetSizesPerHeight(graph, c)}
{
    PushNode(graph.FindRootOfSubGraph(0));
}

void MSTStack::PushNode(size_t vertex)
{
    auto index = GetSize();
    if (index == 0)
        index = GetMaxHeight();

    m_vertices_inside.push_front(m_graph.FindRootOfSubGraph(vertex));
    auto& new_node = m_nodes.emplace_back(m_vertices_inside.cbegin(),
                                          index,
                                          m_sizes_per_height[GetMaxHeight() - index],
                                          m_r);

    AddNewBorderEdgesAfterPush();
    DeleteOldBorderEdgesAfterPush();
}

void MSTStack::AddNewBorderEdgesAfterPush()
{
    auto& new_node = m_nodes.back();

    m_graph.ForEachAvailableEdge([&](const Graph::Details::Edge& edge)
    {
        const auto vertices = edge.GetCurrentSubgraphs(m_graph);

        const auto outside_vertices = vertices | rgv::filter(Utils::IsInRange(m_vertices_inside)) |
                Utils::to_vector;

        if (outside_vertices.empty() || outside_vertices.size() == 2)
            return;

        assert(new_node.GetVertices().size() == 1 && 
            (vertices[0] == new_node.GetVertices().front() || 
             vertices[1] == new_node.GetVertices().front()));

        new_node.PushToHeap(EdgePtrWrapper{ edge });
    });
}

void MSTStack::DeleteOldBorderEdgesAfterPush()
{
    auto& new_node = m_nodes.back();
    assert(new_node.GetVertices().size() == 1);

    auto condition = [&](const EdgePtrWrapper& edge)
    {
        return Utils::IsRangeContains(edge->GetCurrentSubgraphs(m_graph), new_node.GetVertices().front());
    };

    std::for_each_n(m_nodes.cbegin(),
                    m_nodes.size() - 1,
                    [&](SubGraph& node)
                    {
                        const auto old_border_edges = node.DeleteAndReturnIf(condition);
                        if (old_border_edges.empty())
                            return;

                        const auto min = rg::min_element(old_border_edges,
                                                         rg::less{},
                                                         [](const EdgePtrWrapper& ptr)
                                                         {
                                                             return ptr.GetWorkingCost();
                                                         });
                        node.AddToMinLinks(*min);
                    });
}
} // namespace MST::Details
