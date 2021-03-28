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

#include "MSTTreeBuilder.h"

#include <Graph.h>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <spdlog/spdlog.h>

namespace MST
{
MSTTreeBuilder::MSTTreeBuilder(Graph::Details::EdgesView& edges, size_t t)
    : m_graph{graph}
    , m_tree{m_graph, t}
{
    while (true)
    {
        if (m_tree.top().IsMeetTargetSize())
        {
            if (!Retraction())
                return;
        }
        else
        {
            if (!Extension())
                return;
        }
    }
}

bool MSTTreeBuilder::Retraction()
{
    SPDLOG_DEBUG("");

    if (m_tree.top().GetIndex() == 0) // k should be >= 1
        return false;

    PostRetractionActions(m_tree.pop());
    return true;
}

bool MSTTreeBuilder::Extension()
{
    auto extension_edge = FindExtensionEdge();
    SPDLOG_DEBUG("Extension edge {}", !!extension_edge);
    if (!extension_edge)
        return false;

    PostRetractionActions(Fusion(*extension_edge));

    m_tree.push(extension_edge->GetOutsideVertex());

    return true;
}

void MSTTreeBuilder::CreateClustersAndPushCheapest(std::list<Details::EdgePtrWrapper>&& items)
{
    auto vertex = m_tree.top().GetVertices().back();

    std::map<size_t, std::set<Details::EdgePtrWrapper>> clusters_by_out_vertex{};
    std::for_each(std::make_move_iterator(items.begin()),
                  std::make_move_iterator(items.end()),
                  [&](Details::EdgePtrWrapper&& edge)
                  {
                      auto [i,j] = edge->GetCurrentSubgraphs(m_graph);
                      if (i == vertex)
                          clusters_by_out_vertex[j].emplace(edge);
                      else
                      {
                          assert(j == vertex);
                          clusters_by_out_vertex[i].emplace(edge);
                      }
                  });

    for (auto& cluster : clusters_by_out_vertex | rgv::values)
    {
        auto cheapest = cluster.begin();
        for (auto& edge : cluster | rgv::drop(1))
            m_graph.DisableEdge(edge->GetIndex());

        m_tree.top().PushToHeap(*cheapest);
    }
}

Details::EdgePtrWrapper* MSTTreeBuilder::FindExtensionEdge()
{
    auto stack_view = m_tree.view();
    auto transformed_stack_view = rgv::transform(stack_view, &Details::ISubGraph::FindHeapWithMin);
    const auto min_element = rg::min_element(transformed_stack_view,
                                             [](Details::MSTSoftHeapDecorator* left,
                                                Details::MSTSoftHeapDecorator* right)
                                             {
                                                 if (!left)
                                                     return false;
                                                 if (!right)
                                                     return true;
                                                 return *left->FindMin() < *right->FindMin();
                                             });

    auto heap_ptr = *min_element;
    if (!heap_ptr)
        return {};

    return heap_ptr->FindMin(); // don't call DeleteMin, only FindMin! we will remove it later
}

Details::MSTSoftHeapDecorator::ExtractedItems MSTTreeBuilder::Fusion(Details::EdgePtrWrapper& extension_edge)
{
    const auto view = m_tree.view();
    for (auto itr = view.begin(); itr != view.end(); ++itr)
    {
        const auto& min_links = (*itr).GetMinLinks();
        auto edge_itr = rg::find_if(min_links,
                                             [&](const Details::EdgePtrWrapper& edge)
                                             {
                                                 return edge <= extension_edge;
                                             });

        if (edge_itr != min_links.cend())
            return m_tree.fusion(itr.base(), *edge_itr);
    }
    return {};
}

void MSTTreeBuilder::PostRetractionActions(Details::MSTSoftHeapDecorator::ExtractedItems items)
{
    for (auto& corrupted_edge : items.corrupted)
    {
        m_graph.DisableEdge(corrupted_edge->GetIndex());
        m_bad_edges.emplace_back(corrupted_edge->GetIndex());
    }

    CreateClustersAndPushCheapest(std::move(items.items));
}
} // namespace MST