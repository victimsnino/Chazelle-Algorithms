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
TreeNode::Heap* TreeNode::FindMin()
{
    auto min_value = m_heap.FindMin();
    auto min_heap  = &m_heap;
    for (auto& heap : m_cross_heaps)
    {
        if (const auto new_value = heap.FindMin(); !min_value || *new_value < *min_value)
        {
            min_value = new_value;
            min_heap  = &heap;
        }
    }

    return min_value ? min_heap : nullptr;
}

void TreeNode::BuildCrossHeaps(size_t r)
{
    for (size_t i = 0; i < m_label; ++i)
        m_cross_heaps.emplace_back(r, i, m_label);
}

MSTTreeBase::MSTTreeBase(Graph::Graph& graph, size_t c)
    : m_graph(graph)
    , m_r(Utils::CalculateRByEps(1 / static_cast<double>(c)))
    , m_target_sizes_per_height(InitTargetSizesPerHeight(graph, c))
{
    // Now we want to push only one leaf... skip rest part of path
    for (size_t i = 0; i < GetMaxHeight(); ++i)
        m_active_path.emplace_back(m_r, i);

    CreateOneVertexNode(0);
}

TreeNode MSTTreeBase::ContractLastNode()
{
    auto last_subgraph = std::move(m_active_path.back());
    m_active_path.pop_back();

    ContractNode(last_subgraph);

    UpdateActivePathIndexes();

    return last_subgraph;
}

MSTTreeBase::SoftHeap* MSTTreeBase::FindMinAllHeaps() const
{
    auto transformed = m_active_path | std::views::transform(&TreeNode::FindMin);
    const auto itr = std::min_element(transformed.begin(), transformed.end(), 
                                              [](MSTSoftHeapDecorator* left, MSTSoftHeapDecorator* right) -> bool
                                              {
                                                  if (!left)
                                                      return false;

                                                  if (!right)
                                                      return true;

                                                  return left->FindMin() < right->FindMin();
                                              });

    return itr == transformed.end() ? nullptr : *itr;
}


void MSTTreeBase::ContractNode(const TreeNode& node)
{
    // TODO:  DO I NEEED TO UPDATE VALUES OF NODES?
    const auto& vertices = node.GetVertices();
    if (vertices.size() <= 1)
        return;

    std::vector<size_t> edges_to_contract{};

    m_graph.ForEachAvailableEdge([&](const Graph::Details::Edge& edge)
    {
        if (std::ranges::all_of(edge.GetCurrentSubgraphs(m_graph), Utils::IsContainsIn(vertices)))
            edges_to_contract.emplace_back(edge.GetIndex());
    });

    for (const auto edge : edges_to_contract)
        m_graph.ContractEdge(edge);
}

void MSTTreeBase::UpdateActivePathIndexes()
{
    std::ranges::transform(m_vertices_inside_path,
                           m_vertices_inside_path.begin(),
                           [&](size_t i) { return m_graph.FindRootOfSubGraph(i); });

    m_vertices_inside_path.erase(std::unique(m_vertices_inside_path.begin(), m_vertices_inside_path.end()),
                                 m_vertices_inside_path.end());
}

void MSTTreeBase::CreateOneVertexNode(size_t vertex)
{
    vertex = UpdateNodeIndex(vertex);

    m_vertices_inside_path.push_back(vertex);
    auto& node = m_active_path.emplace_back(vertex, m_r, m_active_path.size());

    m_graph.ForEachAvailableEdge([&](Graph::Details::Edge& edge)
    {
        if (Utils::IsContains(edge.GetCurrentSubgraphs(m_graph), vertex))
            node.GetHeap().Insert(edge);
    });
}

bool MSTTreeBase::IsNeedRetraction() const
{
    return m_active_path.size() >= 2 &&
            m_active_path.back().GetVertices().size() >= GetTargetSize(m_active_path.size() - 1);
}
} // namespace MST::Details
