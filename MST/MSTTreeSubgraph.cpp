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

#include "MSTTreeSubgraph.h"

#include <Common.h>


namespace MST::Details
{
SubGraph::SubGraph(size_t vertex, size_t level_in_tree, size_t target_size, size_t r)
    : m_vertex{vertex}
    , m_level_in_tree{level_in_tree}
    , m_target_size{target_size}
{
    m_heaps.reserve(m_level_in_tree+1);

    for (size_t i = 0; i < m_level_in_tree + 1; ++i)
        m_heaps.emplace_back(r);

    m_heaps.shrink_to_fit();
}

void SubGraph::PushToHeap(EdgePtrWrapper edge)
{
    m_heaps[m_level_in_tree].Insert(edge);
}
//
//void SubGraph::AddToMinLinks(EdgePtrWrapper edge)
//{
//    m_min_links_to_next_nodes.emplace_back(edge);
//}
//
//void SubGraph::PopMinLink(bool chain_link)
//{
//    if (!m_min_links_to_next_nodes.empty())
//        m_min_links_to_next_nodes.pop_back();
//}
//
//MSTSoftHeapDecorator* SubGraph::FindHeapWithMin()
//{
//    auto* min_heap  = &m_heaps.front();
//    auto  min_value = min_heap->FindMin();
//    for (auto& heap : m_heaps | rgv::drop(1))
//    {
//        const auto new_value = heap.FindMin();
//        if (!min_value || new_value && *new_value < *min_value)
//        {
//            min_heap  = &heap;
//            min_value = new_value;
//        }
//    }
//    return min_value ? min_heap : nullptr;
//}
//
//std::list<EdgePtrWrapper> SubGraph::DeleteAndReturnIf(std::function<bool(const EdgePtrWrapper& edge)> func)
//{
//    std::list<EdgePtrWrapper> result{};
//    for (auto& heap : m_heaps)
//        result.splice(result.end(), heap.DeleteAndReturnIf(func));
//    return result;
//}
//
//void SubGraph::MeldHeapsFrom(SubGraph& other)
//{
//    // GetIndex() = other.GetIndex() - 1
//    for (size_t i = 0; i < GetIndex(); ++i)
//        m_heaps[i].Meld(other.m_heaps[i]);
//}
//
//MSTSoftHeapDecorator::ExtractedItems SubGraph::ExtractItems()
//{
//    MSTSoftHeapDecorator::ExtractedItems data{};
//
//    std::for_each_n(m_heaps.rbegin(),
//                    std::min(size_t{2}, m_index),
//                    [&](MSTSoftHeapDecorator& heap)
//                    {
//                        auto heap_data = heap.ExtractItems();
//                        data.corrupted.splice(data.corrupted.end(), heap_data.corrupted);
//                        data.items.splice(data.items.end(), heap_data.items);
//                    });
//
//    return data;
//}
//
//void SubGraph::Contract(Graph::Graph& graph, std::list<size_t>& vertices_inside_tree)
//{
//    auto              vertices = GetVertices();
//    std::list<size_t> edges_to_contract{};
//
//    graph.ForEachAvailableEdge([&](const Graph::Details::Edge& edge)
//    {
//        if (rg::all_of(edge.GetCurrentSubgraphs(graph), Utils::IsInRange<decltype(vertices), size_t>(vertices)))
//            edges_to_contract.emplace_back(edge.GetIndex());
//    });
//
//    for (auto index : edges_to_contract)
//        graph.ContractEdge(index);
//
//    const auto first_in_range = std::prev(vertices_inside_tree.erase(std::next(m_vertices_begin), m_vertices_end));
//    *first_in_range           = graph.FindRootOfSubGraph(*first_in_range);
//}
} // namespace MST::Details
