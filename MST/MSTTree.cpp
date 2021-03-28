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

#include "MSTTree.h"

#include "MSTUtils.h"

#include <Common.h>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "MST.h"

#include <spdlog/spdlog.h>

static std::vector<size_t> InitTargetSizesPerHeight(const Graph::Graph& graph, size_t t)
{
    const auto max_height = MST::FindMaxHeight(graph, MST::c);
    std::vector<size_t> out{};

    // Leafs
    out.push_back(1);

    for (uint32_t i = 1; i <= max_height; ++i)
        out.push_back(MST::CalculateTargetSize(t, i));
    return out;
}

namespace MST::Details
{
MSTTree::MSTTree(Graph::Graph& graph, size_t t)
    : m_graph{graph}
    , m_r{Utils::CalculateRByEps(1 / static_cast<double>(MST::c))}
    , m_sizes_per_height{InitTargetSizesPerHeight(graph, t)}
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%s::%!::%#] %v");

    PushNode(graph.FindRootOfSubGraph(0));
}

void MSTTree::push(size_t vertex)
{
    PushNode(m_graph.FindRootOfSubGraph(vertex));
}

MSTSoftHeapDecorator::ExtractedItems MSTTree::pop()
{
    SPDLOG_DEBUG("Size {}", m_active_path.size());

    auto& last_subgraph = m_active_path.back();
    last_subgraph.Contract(m_graph, m_vertices_inside);

    if (m_active_path.size() == 1)
    {
        const size_t index = last_subgraph.GetIndex();
        m_active_path.emplace_front(m_vertices_inside.cbegin(),
                              index - 1,
                              m_sizes_per_height[IndexToHeight(index - 1)],
                              m_r);
    }

    std::next(m_active_path.rbegin())->MeldHeapsFrom(last_subgraph);
    auto data = last_subgraph.ExtractItems();

    m_active_path.pop_back();

    std::for_each_n(m_active_path.begin(),
                    m_active_path.size() - 1,
                    [](SubGraph& graph) { graph.PopMinLink(); });

    m_active_path.back().PopMinLink(true);

    return data;
}

MSTSoftHeapDecorator::ExtractedItems MSTTree::fusion(std::list<SubGraph>::iterator itr,
                                                      const EdgePtrWrapper&         fusion_edge)
{
    auto pop_count = std::distance(itr, m_active_path.end()) - 1;
    SPDLOG_DEBUG("pop_count {}",pop_count);

    MSTSoftHeapDecorator::ExtractedItems items{};
    for (size_t i = 0; i < pop_count; ++i)
    {
        auto data = pop();
        items.corrupted.splice(items.corrupted.end(), data.corrupted);
        items.items.splice(items.items.end(), data.items);
    }

    m_graph.ContractEdge(fusion_edge->GetIndex());
    m_vertices_inside.pop_back();
    return items;
}

void MSTTree::PushNode(size_t vertex)
{
    SPDLOG_DEBUG(vertex);

    auto index = size();
    // Empty stack, initialization stage
    if (index == 0)
        index = GetMaxHeight();

    m_vertices_inside.push_back(vertex);
    m_active_path.emplace_back(std::prev(m_vertices_inside.cend()),
                         index,
                         m_sizes_per_height[IndexToHeight(index)],
                         m_r);

    AddNewBorderEdgesAfterPush();
    DeleteOldBorderEdgesAndUpdateMinLinksAfterPush();
}

void MSTTree::AddNewBorderEdgesAfterPush()
{
    auto& new_node = m_active_path.back();

    m_graph.ForEachAvailableEdge([&](const Graph::Details::Edge& edge)
    {
        const auto vertices = edge.GetCurrentSubgraphs(m_graph);

        const auto outside_vertices = vertices |
                rgv::filter(std::not_fn(Utils::IsInRange(m_vertices_inside))) |
                Utils::to_vector;

        if (outside_vertices.empty() || outside_vertices.size() == 2)
            return;

        if ((vertices[0] == outside_vertices.front() ? vertices[1] : vertices[0]) == new_node.GetVertex())
            new_node.PushToHeap(EdgePtrWrapper{edge, outside_vertices.front()});
    });
}

void MSTTree::DeleteOldBorderEdgesAndUpdateMinLinksAfterPush()
{
    auto& new_node = m_active_path.back();
    if (m_active_path.size() <= 1)
        return;

    auto condition = [&](const EdgePtrWrapper& edge)
    {
        return Utils::IsRangeContains(edge->GetCurrentSubgraphs(m_graph), new_node.GetVertex());
    };

    std::for_each_n(m_active_path.begin(),
                    m_active_path.size() - 1,
                    [&](SubGraph& node)
                    {
                        const auto old_border_edges = node.DeleteAndReturnIf(condition);
                        if (old_border_edges.empty())
                            return;

                        auto min = std::min_element(old_border_edges.cbegin(),
                                                    old_border_edges.cend(),
                                                    [](const EdgePtrWrapper& left, const EdgePtrWrapper& right)
                                                    {
                                                        return left.GetWorkingCost() < right.GetWorkingCost();
                                                    });
                        node.AddToMinLinks(*min);
                    });
}
} // namespace MST::Details
