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

#include <Common.h>


#include <iostream>
#include <ranges>
#include <stdexcept>

static constexpr uint32_t S(uint32_t i, uint32_t j)
{
    if (i == 0 || j == 0)
        throw std::out_of_range{"i and j must be > 0"};

    if (i == 1)
        return 2 * j;
    if (j == 1)
        return 2;
    return S(i, j - 1) * S(i - 1, S(i, j - 1));
}

static_assert(S(1, 5) == 2 * 5);
static_assert(S(100500, 1) == 2);

static uint32_t FindMaxHeight(const Graph::Graph& graph, uint32_t c)
{
    const double vertexes_count = static_cast<double>(graph.GetVertexesCount());
    const double edges_count    = static_cast<double>(graph.GetEdgesCount());
    return c * static_cast<uint32_t>(std::ceil(std::pow(edges_count / vertexes_count, 1.0 / 3.0)));
}

static uint32_t FindParamT(const Graph::Graph& graph, uint32_t d)
{
    if (d == 1)
        return 1;

    const double vertexes_count = static_cast<double>(graph.GetVertexesCount());

    uint32_t result = 1;
    while (vertexes_count > std::pow(S(result, d), 3))
        ++result;

    return result;
}

static uint32_t CalculateTargetSize(uint32_t t, uint32_t node_height)
{
    if (node_height == 1)
        return std::pow(S(t, 1u), 3u);
    return std::pow(S(t - 1, S(t, node_height - 1)), 3u);
}


namespace MST
{
MSTTree::MSTTree(Graph::Graph& graph, size_t c)
    : m_graph(graph)
    , m_max_height(FindMaxHeight(graph, c))
    , m_t(FindParamT(graph, m_max_height) + 1) // TODO
    , m_r(Utils::CalculateRByEps(1 / static_cast<double>(c)))
{
    std::cout << "d " << m_max_height << " t " << m_t << std::endl;

    // TODO
    //if (!BoruvkaPhase(c)) 
    //    return;

    InitiateTree();

    BuildTree();
}


bool MSTTree::BoruvkaPhase(size_t c) const
{
    size_t count = m_t == 1 ? std::numeric_limits<uint32_t>::max() : c;

    while (count > 0 && m_graph.GetVertexesCount() > 1)
    {
        m_graph.BoruvkaPhase();
        --count;
    }

    if (m_graph.GetVertexesCount() == 1)
        return false;
    return true;
}


void MSTTree::InitiateTree()
{
    // Leafs
    m_target_sizes_per_height.push_back(1);

    for (uint32_t i = 1; i <= m_max_height; ++i)
        m_target_sizes_per_height.push_back(CalculateTargetSize(m_t, i));

    for (size_t i = 0; i < m_max_height; ++i)
        m_active_path.push({m_r});

    CreateOneVertexNode(0);
}


void MSTTree::BuildTree()
{
    while (true)
    {
        if (m_active_path.size() >= 2 &&
            m_active_path.top().GetCountOfVertices() >= GetTargetSize(IndexToHeight(m_active_path.size() - 1)))
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
    auto last_subgraph = std::move(m_active_path.top());
    m_active_path.pop();

    ContractNode(last_subgraph);

    auto k = m_active_path.size();

    std::list<Edge> valid_items{};
    for (auto& heap : {&last_subgraph.GetHeap(), &GetHeap(k - 1, k)})
    {
        auto extracted = heap->ExtractItems();
        for (const auto& edge : extracted.corrupted)
            m_graph.DisableEdge(edge.GetIndex());

        valid_items.splice(valid_items.end(), extracted.items);
    }

    const auto ret = std::ranges::unique(m_vertices_inside_path,
                                         std::ranges::equal_to{},
                                         std::bind(&Graph::Graph::FindRootOfSubGraph, &m_graph, std::placeholders::_1));

    m_vertices_inside_path.erase(ret.begin(), ret.end());
    CreateClusters(k, std::move(valid_items));
}


void MSTTree::ContractNode(const TreeNode& last_subgraph)
{
    // TODO:  DO I NEEED TO UPDATE VALUES OF NODES?
    const auto& vertices = last_subgraph.GetVertices();
    if (vertices.size() <= 1)
        return;

    std::vector<size_t> edges_to_contract{};

    m_graph.ForEachAvailableEdge([&](const Edge& edge)
    {
        if (std::ranges::all_of(edge.GetCurrentSubgraphs(m_graph), Utils::IsContainsIn(vertices)))
            edges_to_contract.emplace_back(edge.GetIndex());
    });

    for (const auto edge : edges_to_contract)
        m_graph.ContractEdge(edge);
}


void MSTTree::InsertEdgeToLastNodeHeap(Edge& edge)
{
    m_active_path.top().GetHeap().Insert(edge);
    edge.SaveLastHeapIndex(m_active_path.size() - 1);
}


void MSTTree::CreateClusters(size_t k, std::list<Edge>&& valid_items)
{
    std::map<size_t, Cluster> out_vertex_to_internal{};

    std::ranges::for_each(valid_items,
                          [&](Edge& edge)
                          {
                              auto subgraphs = edge.GetCurrentSubgraphs(m_graph);
                              if (std::ranges::all_of(subgraphs, Utils::IsContainsIn(m_vertices_inside_path)))
                                  return;

                              auto [i, j] = subgraphs;
                              if (Utils::IsContains(m_vertices_inside_path, i))
                                  out_vertex_to_internal[j].emplace(edge);
                              else if (Utils::IsContains(m_vertices_inside_path, j))
                                  out_vertex_to_internal[i].emplace(edge);
                          });

    for (auto& [out_vertex, edges] : out_vertex_to_internal)
    {
        const auto cheapest_edge_ref = edges.begin();
        edges.erase(cheapest_edge_ref);

        for (const auto& edge : edges)
            m_graph.DisableEdge(edge.get().GetIndex());

        InsertToHeapForEdge(cheapest_edge_ref->get(), edges, k);
    }
}


void MSTTree::InsertToHeapForEdge(const Edge&    edge,
                                  const Cluster& his_cluster,
                                  size_t         k)
{
    auto [i, j] = edge.GetLastHeapIndex();

    assert(i.has_value());

    auto indexes_view = his_cluster | std::views::transform(&Edge::GetLastHeapIndex);

    if (i == k - 1 && j == k || i == k && !j.has_value()
        && Utils::IsContains(indexes_view, std::array<std::optional<size_t>, 2>{k - 1, k}))
    {
        assert(m_active_path.size() == k);
        m_active_path.top().GetHeap(); // H(k-1)
    }
    assert(i == k && !j.has_value());

    auto index_to_insert = std::ranges::find_if(indexes_view,
                                                [&](const std::array<std::optional<size_t>, 2>& indexes)
                                                {
                                                    return indexes[1] == k;
                                                });
    if (index_to_insert == std::cend(indexes_view))
        throw std::out_of_range{"Cant'f find suitable heap"};

    GetHeap((*index_to_insert)[0].value() ,k);
}


void MSTTree::CreateOneVertexNode(size_t vertex)
{
    vertex = m_graph.FindRootOfSubGraph(vertex);

    m_vertices_inside_path.push_back(vertex);
    auto& node = m_active_path.emplace(vertex);

    m_graph.ForEachAvailableEdge([&](Edge& edge)
    {
        if (Utils::IsContains(edge.GetCurrentSubgraphs(m_graph), vertex))
            InsertEdgeToLastNodeHeap(edge);
    });
}


uint32_t MSTTree::GetTargetSize(uint32_t node_height) const
{
    return m_target_sizes_per_height[node_height];
}


uint32_t MSTTree::IndexToHeight(uint32_t index) const
{
    return m_max_height - index - 1;
}

MSTTree::SoftHeap& MSTTree::GetHeap(size_t i, size_t j)
{
    auto& heaps = m_heaps[i];
    return heaps.emplace(j, m_r).first->second;
}
}
