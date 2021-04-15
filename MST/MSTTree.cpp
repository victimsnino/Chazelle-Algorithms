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

#include "MST.h"
#include "MSTUtils.h"

#include <Common.h>

#include <exception>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <spdlog/spdlog.h>

#include <deque>

static std::vector<size_t> InitTargetSizesPerHeight(size_t t, size_t max_height)
{
    std::vector<size_t> out{};

    // Leafs
    out.push_back(1);

    for (uint32_t i = 1; i <= max_height; ++i)
        out.push_back(::MST::CalculateTargetSize(t, i));
    return out;
}

namespace MST::Details
{
MSTTree::MSTTree(Graph::Details::EdgesView& edges, size_t t, size_t max_height)
    : m_edges{edges}
    , m_r{Utils::CalculateRByEps(1 / static_cast<double>(MST::c))}
    , m_sizes_per_height{InitTargetSizesPerHeight(t, max_height)}
{
    const auto itr = std::find_if(m_edges.begin(),
                                  m_edges.end(),
                                  [](const Graph::Details::Edge& edge)
                                  {
                                      return !edge.IsDisabled() && !edge.IsContracted();
                                  });

    if (itr == m_edges.end())
        throw std::exception{"Graph without edges!"};

    PushNode((*itr).GetCurrentSubgraphs()[0]);
}

void MSTTree::push(const EdgePtrWrapper& extension_edge)
{
    SPDLOG_DEBUG("Push with edge {}", extension_edge->GetIndex());

    auto& pre_last = m_active_path.back();
    PushNode(extension_edge.GetOutsideVertex());

    pre_last->AddChild(extension_edge->GetIndex(), m_active_path.back());
}

MSTSoftHeapDecorator::ExtractedItems MSTTree::pop()
{
    SPDLOG_DEBUG("Size {}", m_active_path.size());

    auto& last_subgraph = m_active_path.back();

    if (m_active_path.size() == 1)
    {
        m_active_path.emplace_front(std::make_shared<SubGraph>(last_subgraph,
                                    m_sizes_per_height[IndexToHeight(last_subgraph->GetLevelInTree() - 1)],
                                    m_r));
    }

    (*std::next(m_active_path.rbegin()))->MeldHeapsFrom(last_subgraph);
    auto data = last_subgraph->ExtractItems();

    m_active_path.pop_back();

    std::for_each_n(m_active_path.begin(),
                    m_active_path.size(),
                    [](SubGraphPtr& graph) { graph->PopMinLink(); });

    return data;
}

MSTSoftHeapDecorator::ExtractedItems MSTTree::fusion(std::list<SubGraphPtr>::iterator itr,
                                                     const EdgePtrWrapper&            fusion_edge)
{
    auto pop_count = std::distance(itr, m_active_path.end()) - 1;
    SPDLOG_DEBUG("pop_count for fusion {}", pop_count);

    MSTSoftHeapDecorator::ExtractedItems items{};
    for (size_t i = 0; i < pop_count; ++i)
    {
        auto data = pop();
        items.corrupted.splice(items.corrupted.end(), data.corrupted);
        items.items.splice(items.items.end(), data.items);
    }

    // Fused chain doesn't become an vertex of this subgraph
    auto last_child = m_active_path.back()->PopLastChild();

    auto [i,j] = fusion_edge->GetCurrentSubgraphs();
    auto childs = m_active_path.back()->GetChilds();
    auto child_itr = rg::find_if(childs, [&](const SubGraphPtr& sub)
    {
        auto vertices = sub->GetVertices();
        return Utils::IsRangeContains(vertices, i) ||  Utils::IsRangeContains(vertices, j); 
    });

    assert(child_itr != childs.end());

    (*child_itr)->AddChild(fusion_edge->GetIndex(), last_child);
    return items;
}

ISubGraph& MSTTree::top()
{
    assert(!m_active_path.empty());
    return *m_active_path.back();
}

size_t MSTTree::size() const { return m_active_path.empty() ? 0 : m_active_path.back()->GetLevelInTree() + 1; }

std::vector<Graph::Graph> MSTTree::CreateSubGraphs(const std::vector<size_t>& bad_edges)
{
    std::list list_of_subgraphs{m_active_path.front()};
    std::vector<Graph::Graph> result{};
    while(!list_of_subgraphs.empty())
    {
        auto& front = list_of_subgraphs.front();
        auto& graph = result.emplace_back();

        for (const auto& edge_index : front->GetChildsEdges())
        {
            if (Utils::IsRangeContains(bad_edges, edge_index))
                continue;

            auto& edge  = m_edges[edge_index];
            auto  [i,j] = edge.GetCurrentSubgraphs();
            graph.AddEdge(i, j, edge.GetWeight(), edge.GetIndex());
        }

        for(auto& child : front->GetChilds())
        {
            list_of_subgraphs.emplace_back(child);

            auto vertices = child->GetVertices(true);
            auto front = vertices.front();
            for(auto other : vertices | rgv::drop(1))
                graph.UnionVertices(front, other);
        }
        if (graph.GetVerticesCount() <= 1)
            result.pop_back();

        // DUPLICATED FIRST SUBGRAPH!
        list_of_subgraphs.pop_front();
    }
    return result;
}

std::list<size_t> MSTTree::GetVerticesInside()
{
    return m_active_path.front()->GetVertices(true);
}

void MSTTree::PushNode(size_t vertex)
{
    SPDLOG_DEBUG(vertex);

    auto index = size();
    // Empty stack, initialization stage
    if (index == 0)
        index = GetMaxHeight();

    m_active_path.emplace_back(std::make_shared<SubGraph>(vertex,
                               index,
                               m_sizes_per_height[IndexToHeight(index)],
                               m_r));

    AddNewBorderEdgesAfterPush();
    DeleteOldBorderEdgesAndUpdateMinLinksAfterPush();
}

void MSTTree::AddNewBorderEdgesAfterPush()
{
    auto& new_node      = m_active_path.back();
    auto  node_vertices = new_node->GetVertices();
    auto all_vertices = m_active_path.front()->GetVertices();

    assert(node_vertices.size() == 1);

    for (const auto& edge : m_edges)
    {
        const auto [i,j] = edge.GetCurrentSubgraphs();
        if (Utils::IsRangeContains(node_vertices, i))
        {
            if (!Utils::IsRangeContains(all_vertices, j))
                new_node->PushToHeap(EdgePtrWrapper{edge, j});
        }
        else if (Utils::IsRangeContains(node_vertices, j))
        {
            if (!Utils::IsRangeContains(all_vertices, i))
                new_node->PushToHeap(EdgePtrWrapper{edge, i});
        }
    }
}

void MSTTree::DeleteOldBorderEdgesAndUpdateMinLinksAfterPush()
{
    auto& new_node = m_active_path.back();
    if (m_active_path.size() <= 1)
        return;

    auto condition = [&](const EdgePtrWrapper& edge)
    {
        return Utils::IsRangeContains(edge->GetCurrentSubgraphs(), new_node->GetVertices().back());
    };

    std::for_each_n(m_active_path.begin(),
                    m_active_path.size() - 1,
                    [&](SubGraphPtr& node)
                    {
                        const auto old_border_edges = node->DeleteAndReturnIf(condition);
                        if (old_border_edges.empty())
                            return;

                        auto min = std::min_element(old_border_edges.cbegin(),
                                                    old_border_edges.cend());
                        node->AddToMinLinks(*min);
                    });
}
} // namespace MST::Details
