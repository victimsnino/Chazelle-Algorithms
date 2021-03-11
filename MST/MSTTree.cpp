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
    auto       contracted_last_node = m_base.ContractLastNode();
    const auto k                    = contracted_last_node.GetLabel();
    auto&      current_last         = m_base.GetLastNode(); // k-1

    current_last.AddVertex(m_base.UpdateNodeIndex(contracted_last_node.GetVertices()[0]));

    std::list<Details::EdgePtrWrapper> valid_items{};
    for (auto& heap_ptr : {&contracted_last_node.GetHeap(),
                           &contracted_last_node.GetCrossHeapFrom(k - 1)})
    {
        auto [corrupted, items] = heap_ptr->ExtractItems();
        for (const auto& edge : corrupted)
            m_base.GetGraph().DisableEdge(edge->GetIndex());

        valid_items.splice(valid_items.end(), items);
    }

    MoveItemsToSuitableHeapsByClusters(contracted_last_node.GetCrossHeaps(), std::move(valid_items));

    for (size_t i = 0; i < k - 1; ++i)
        current_last.GetCrossHeapFrom(i).Meld(contracted_last_node.GetCrossHeapFrom(i));
}

void MSTTree::MoveItemsToSuitableHeapsByClusters(std::vector<Details::MSTSoftHeapDecorator>& cross_heaps,
                                                 std::list<Details::EdgePtrWrapper>&&              valid_items)
{
    std::map<size_t, Cluster> out_vertex_to_internals{};
    std::ranges::for_each(valid_items,
                          [&](Graph::Details::Edge& edge)
                          {
                              auto subgraphs = edge.GetCurrentSubgraphs(m_base.GetGraph());
                              if (std::ranges::all_of(subgraphs, Utils::IsContainsIn(m_base.GetVerticesInsidePath())))
                                  return;

                              auto [i, j] = subgraphs;
                              if (Utils::IsContains(m_base.GetVerticesInsidePath(), i))
                                  out_vertex_to_internals[j].emplace(edge);
                              else if (Utils::IsContains(m_base.GetVerticesInsidePath(), j))
                                  out_vertex_to_internals[i].emplace(edge);
                          },
                          &Details::EdgePtrWrapper::GetEdge);

    for (auto& edges : out_vertex_to_internals | std::views::values)
    {
        const auto cheapest_edge_ref = edges.begin();
        auto       dropped_view      = edges | std::views::drop(1);
        for (const auto& edge : dropped_view)
            m_base.GetGraph().DisableEdge(edge->GetIndex());

        InsertToHeapForEdge(cheapest_edge_ref->GetEdge(), dropped_view.empty(), cross_heaps);
    }
}

void MSTTree::InsertToHeapForEdge(Graph::Details::Edge&                             edge,
                                  bool                                              is_his_cluster_empty,
                                  std::vector<Details::MSTSoftHeapDecorator>& cross_heaps)
{
    auto [i, j]   = edge.GetLastHeapIndex();
    auto [v1, v2] = edge.GetCurrentSubgraphs(m_base.GetGraph());

    auto is_contains_same_vertex_fn = [&](const Details::EdgePtrWrapper& temp_edge) -> bool
    {
        auto [tv1, tv2] = temp_edge->GetCurrentSubgraphs(m_base.GetGraph());
        return tv1 == v1 || tv1 == v2 || tv2 == v1 || tv2 == v2;
    };
    assert(i.has_value());
    assert(i == m_base.GetLastNode().GetLabel() && j == m_base.GetLastNode().GetLabel() + 1
        || !j.has_value() && i == m_base.GetLastNode().GetLabel() + 1);

    if (!j.has_value() // from the internal heap, not cross, so it is from H(k)
        && std::ranges::find_if(cross_heaps[i.value() - 1].GetItemsInside(), 
            is_contains_same_vertex_fn) != cross_heaps[i.value() - 1].GetItemsInside().end()
        || i.value() + 1 == j // H(k-1, k)
        || is_his_cluster_empty)
    {
        m_base.GetLastNode().GetHeap().Insert(edge); // H(k-1)
        return;
    }

    for (auto& cross_heap : cross_heaps)
    {
        const auto& items_in_heap = cross_heap.GetItemsInside();
        if (std::ranges::find_if(items_in_heap, is_contains_same_vertex_fn) == items_in_heap.end())
            return;

        cross_heap.Insert(edge);
        return;
    }

    throw std::out_of_range{"Cant'f find suitable heap"};
}
} // namespace MST
