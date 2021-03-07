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
MSTTreeBase::MSTTreeBase(Graph::Graph& graph, size_t c)
    : m_graph(graph)
    , m_r(Utils::CalculateRByEps(1 / static_cast<double>(c)))
    , m_target_sizes_per_height(InitTargetSizesPerHeight(graph, c))
{
    // Now we want to push only one leaf... skip rest part of path
    for (size_t i = 0; i < GetMaxHeight(); ++i)
        m_active_path.push({m_r, i});

    CreateOneVertexNode(0);
}

MSTTreeBase::SoftHeap::ExtractedItems MSTTreeBase::ContractLastAddToNextAndExtractEdgesFromHeap()
{
    auto k = GetIndexOfLastInPath();

    auto last_subgraph = std::move(m_active_path.top());
    m_active_path.pop();

    ContractNode(last_subgraph);

    m_active_path.top().AddVertex(UpdateNodeIndex(last_subgraph.GetVertices()[0]));

    UpdateActivePathIndexes();

    auto extracted      = last_subgraph.GetHeap().ExtractItems();
    auto heap_extracted = GetHeap(k - 1, k).ExtractItems();

    m_cross_heaps[k - 1].erase(k);

    extracted.items.splice(extracted.items.end(), heap_extracted.items);
    extracted.corrupted.splice(extracted.corrupted.end(), heap_extracted.corrupted);

    return extracted;
}

void MSTTreeBase::MeldHeapsFromTo(std::array<size_t, 2> from, std::array<size_t, 2> to)
{
    auto heaps_from = m_cross_heaps[from[0]];
    if (heaps_from.find(from[1]) == heaps_from.cend())
        return;

    GetHeap(to[0], to[1]).Meld(GetHeap(from[0], from[1]));
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
    auto& node = m_active_path.emplace(vertex, m_active_path.size());

    m_graph.ForEachAvailableEdge([&](Graph::Details::Edge& edge)
    {
        if (Utils::IsContains(edge.GetCurrentSubgraphs(m_graph), vertex))
            node.GetHeap().Insert(edge);
    });
}

size_t MSTTreeBase::GetTargetSize(size_t node_index) const
{
    return m_target_sizes_per_height[GetMaxHeight() - node_index];
}

MSTTreeBase::SoftHeap& MSTTreeBase::GetLastNodeHeap()
{
    return m_active_path.top().GetHeap();
}

bool MSTTreeBase::IsCanRetraction() const
{
    return m_active_path.size() >= 2 &&
            m_active_path.top().GetVertices().size() >= GetTargetSize(m_active_path.size() - 1);
}

MSTTreeBase::SoftHeap& MSTTreeBase::GetHeap(size_t i, size_t j)
{
    auto& heaps = m_cross_heaps[i];
    return heaps.try_emplace(j, m_r, i, j ).first->second;
}
} // namespace MST::Details
