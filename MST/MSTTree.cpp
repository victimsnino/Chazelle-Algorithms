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

#include <iostream>
#include <ranges>
#include <stdexcept>


namespace MST
{
MSTTree::MSTTree(Graph::Graph& graph, size_t c)
    : m_base{graph, c}
{
    if (!BoruvkaPhase(c)) 
        return;

    BuildTree();
}

bool MSTTree::BoruvkaPhase(size_t c) const
{
    // TODO: ENable borubka Phase later
   /* size_t count = FindParamT(graph, FindMaxHeight(m_base.GetGraph(), c)) == 1 ? std::numeric_limits<uint32_t>::max() : c;

    while (count > 0 && m_graph.GetVertexesCount() > 1)
    {
        m_graph.BoruvkaPhase();
        --count;
    }

    if (m_graph.GetVertexesCount() == 1)
        return false;*/
    return true;
}

void MSTTree::BuildTree()
{
    while (true)
    {
        if (m_base.IsCanRetraction())
        {
            Retraction();
        }
        else
        {
            if (!Extension())
                return;
        }
    }
}

bool MSTTree::Extension()
{
    return false;
}

void MSTTree::Retraction()
{
    const size_t k                  = m_base.GetIndexOfLastInPath();
    auto         [corrupted, items] = m_base.ContractLastAddToNextAndExtractEdgesFromHeap();

    for (const auto& edge : corrupted)
        m_base.GetGraph().DisableEdge(edge->GetIndex());

    MoveItemsToSuitableHeapsByClusters(k, std::move(items));
    for (size_t i = 0; i < k - 1; ++i)
        m_base.MeldHeapsFromTo({ i, k }, { i, k - 1 });
}

void MSTTree::MoveItemsToSuitableHeapsByClusters(size_t k, std::list<Details::EdgePtrWrapper>&& valid_items)
{
    std::map<size_t, Cluster> out_vertex_to_internal_for_heaps{};
    std::ranges::for_each(valid_items | std::views::transform(&Details::EdgePtrWrapper::GetEdge),
                              CreateClustersFunctor(out_vertex_to_internal_for_heaps));

    std::map<size_t, Cluster> out_vertex_to_internal{};
    m_base.GetGraph().ForEachAvailableEdge(CreateClustersFunctor(out_vertex_to_internal));

    for (auto& [out_vertex, edges] : out_vertex_to_internal_for_heaps)
    {
        const auto cheapest_edge_ref = edges.begin();
        for (const auto& edge : edges | std::views::drop(1))
            m_base.GetGraph().DisableEdge(edge->GetIndex());

        InsertToHeapForEdge(cheapest_edge_ref->GetEdge(),
                            out_vertex_to_internal[out_vertex],
                            k);
    }
}

void MSTTree::InsertToHeapForEdge(Graph::Details::Edge& edge,
                                  const Cluster&        his_cluster,
                                  size_t                k)
{
    auto [i, j] = edge.GetLastHeapIndex();

    assert(i.has_value());

    auto indexes_view = his_cluster |
            std::views::transform(&Details::EdgePtrWrapper::GetEdge) |
            std::views::filter([&](const Graph::Details::Edge& tmp)
            {
                return tmp != edge;
            }) |
            std::views::transform(&Graph::Details::Edge::GetLastHeapIndex);

    if (i == k - 1 && j == k || 
        i == k && !j.has_value() &&
        (std::ranges::count(indexes_view, std::array<std::optional<size_t>, 2>{k - 1, k}) != 0 ||
            his_cluster.size() <= 1))
    {
        m_base.GetLastNodeHeap().Insert(edge); // H(k-1)
        return;
    }

    for (const auto& indexes : indexes_view)
    {
        if (indexes[1] != k)
            continue;

        m_base.GetHeap(indexes[0].value(), k).Insert(edge);
        return;
    }

    throw std::out_of_range{"Cant'f find suitable heap"};
}

std::function<void(Graph::Details::Edge&)> MSTTree::CreateClustersFunctor(std::map<size_t, Cluster>& out)
{
    return [&](Graph::Details::Edge& edge)
    {
        auto subgraphs = edge.GetCurrentSubgraphs(m_base.GetGraph());
        if (std::ranges::all_of(subgraphs, Utils::IsContainsIn(m_base.GetVerticesInsidePath())))
            return;

        auto [i, j] = subgraphs;
        if (Utils::IsContains(m_base.GetVerticesInsidePath(), i))
            out[j].emplace(edge);
        else if (Utils::IsContains(m_base.GetVerticesInsidePath(), j))
            out[i].emplace(edge);
    };
}
}
