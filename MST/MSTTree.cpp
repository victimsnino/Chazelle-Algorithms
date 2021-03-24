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

#include <Graph.h>


namespace MST
{
MSTTree::MSTTree(Graph::Graph& graph, size_t c)
    : m_graph{graph}
    , m_stack{m_graph, c}
{
    while (true)
    {
        if (m_stack.top().IsMeetTargetSize())
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

bool MSTTree::Retraction()
{
    if (m_stack.top().GetIndex() == 0) // k should be >= 1
        return false;

    auto [corrupted, items] = m_stack.pop();
    for (auto& corrupted_edge : corrupted)
        m_graph.DisableEdge(corrupted_edge->GetIndex());

    CreateClustersAndPushCheapest(std::move(items));

    return true;
}

bool MSTTree::Extension()
{
    auto extension_edge = FindExtensionEdge();
    if (!extension_edge)
        return false;

    // TODO: Fusion
    // Fusion();

    m_stack.push(extension_edge->GetOutsideVertex());

    return true;
}

void MSTTree::CreateClustersAndPushCheapest(std::list<Details::EdgePtrWrapper>&& items)
{
    auto vertex = m_stack.top().GetVertices().back();

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

        m_stack.top().PushToHeap(*cheapest);
    }
}

Details::EdgePtrWrapper* MSTTree::FindExtensionEdge()
{
    auto stack_view = m_stack.view();
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

MSTTree MSTTree::Create(Graph::Graph& graph, size_t c)
{
    // TODO: ENable boruvka Phase later
    //size_t count = FindParamT(graph, FindMaxHeight(graph, c)) == 1 ? std::numeric_limits<uint32_t>::max() : c;

    //while (count > 0 && graph.GetVertexesCount() > 1)
    //{
    //    graph.BoruvkaPhase();
    //    --count;
    //}

    //if (graph.GetVertexesCount() == 1)
    //    return false;
    //return true;
    return MSTTree{graph, c};
}
} // namespace MST
