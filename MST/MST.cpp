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

#include "MST.h"

#include "Common.h"


#include <Graph.h>
#include <SoftHeapCpp.h>

#include <iostream>
#include <stack>
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
template<size_t c>
class MSTTree
{
public:
    MSTTree(Graph::Graph& graph);

private:
    class TreeNode
    {
    public:
        TreeNode() = default;
        TreeNode(size_t vertex) { AddVertex(vertex); }

        void  AddVertex(size_t vertex) { m_vertices.push_back(vertex); }
        auto& GetHeap() { return m_heap; }

        size_t                     GetCountOfVertices() const { return m_vertices.size(); }
        const std::vector<size_t>& GetVertices() const { return m_vertices; }
    private:
        std::vector<size_t>                      m_vertices{};
        SoftHeapCpp<Graph::Details::Edge> m_heap{Utils::CalculateRByEps(1 / static_cast<double>(c))};
    };

private:
    // Step 1,2: Boruvka
    bool BoruvkaPhase() const;

    // Preparation for step 3
    void InitiateTree();

    // Step 3: Building tree T
    void BuildTree();

private:
    bool Extension();
    void Retraction();

    void CreateClusters();

    void CreateOneVertexNode(size_t vertex);
    void ContractNode(const TreeNode& last_subgraph);

    uint32_t GetTargetSize(uint32_t node_height) const;
    uint32_t IndexToHeight(uint32_t index) const;

private:
    Graph::Graph&         m_graph;
    const uint32_t        m_max_height;
    const uint32_t        m_t;
    std::vector<uint32_t> m_target_sizes_per_height{};
    std::stack<TreeNode>  m_active_path{};
    std::vector<size_t>   m_vertices_inside_path{};
};

template<size_t c>
MSTTree<c>::MSTTree(Graph::Graph& graph)
    : m_graph(graph)
    , m_max_height(FindMaxHeight(graph, c))
    , m_t(FindParamT(graph, m_max_height) + 1) // TODO
{
    std::cout << "d " << m_max_height << " t " << m_t << std::endl;

    // TODO
    //if (!BoruvkaPhase()) 
    //    return;

    InitiateTree();

    BuildTree();
}

template<size_t c>
bool MSTTree<c>::BoruvkaPhase() const
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

template<size_t c>
void MSTTree<c>::InitiateTree()
{
    // Leafs
    m_target_sizes_per_height.push_back(1);

    for (uint32_t i = 1; i <= m_max_height; ++i)
        m_target_sizes_per_height.push_back(CalculateTargetSize(m_t, i));

    for (size_t i = 0; i < m_max_height; ++i)
        m_active_path.push({});

    CreateOneVertexNode(0);
}

template<size_t c>
void MSTTree<c>::BuildTree()
{
    while (true)
    {
        if (m_active_path.size() >= 2 &&
            m_active_path.top().GetCountOfVertices() >= GetTargetSize(IndexToHeight(m_active_path.size()-1)))
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

template<size_t c>
bool MSTTree<c>::Extension()
{
}

template<size_t c>
void MSTTree<c>::Retraction()
{
    auto&& last_subgraph = std::move(m_active_path.top());
    m_active_path.pop();

    ContractNode(last_subgraph);
    last_subgraph.GetHeap()
}

template<size_t c>
void MSTTree<c>::ContractNode(const TreeNode& last_subgraph)
{
    // TODO:  DO I NEEED TO UPDATE VALUES?
    const auto& vertices = last_subgraph.GetVertices();
    if (vertices.size() <= 1)
        return;

    std::vector<size_t>                                          edges_to_contract{};
    m_graph.ForEachAvailableEdge([&](const Graph::Details::Edge& edge)
    {
        const auto subgraphs = edge.GetCurrentSubgraphs(m_graph);
        if (std::all_of(subgraphs.cbegin(),
                        subgraphs.cend(),
                        [&](size_t vertex)
                        {
                            return std::find(vertices.cbegin(), vertices.cend(), vertex) != vertices.cend();
                        }))
            edges_to_contract.emplace_back(edge.GetIndex());
    });

    for (const auto edge : edges_to_contract)
        m_graph.ContractEdge(edge);
}

template<size_t c>
void MSTTree<c>::CreateClusters()
{
    std::map<size_t, std::set<std::reference_wrapper<const Graph::Details::Edge>>> out_vertex_to_internal{};

    m_graph.ForEachAvailableEdge([&](const Graph::Details::Edge& edge)
    {
        auto [i, j] = edge.GetCurrentSubgraphs(m_graph);
        if (std::find(m_vertices_inside_path.cbegin(), m_vertices_inside_path.cend(), i) != m_vertices_inside_path.cend())
        {
            out_vertex_to_internal[j].emplace(edge);
        }
        else if (std::find(m_vertices_inside_path.cbegin(), m_vertices_inside_path.cend(), j) != m_vertices_inside_path.cend())
        {
            out_vertex_to_internal[i].emplace(edge);
        }
    });

    for (auto& [out_vertex, edges] : out_vertex_to_internal)
    {
        const auto cheapest_edge_ref = edges.begin();
        edges.erase(cheapest_edge_ref);

        for (const auto& edge : edges)
            m_graph.DisableEdge(edge.get().GetIndex());

        // TODO
        //GetHeap().insert(cheapest_edge_ref);
    }
}

template<size_t c>
void MSTTree<c>::CreateOneVertexNode(size_t vertex)
{
    vertex = m_graph.FindRootOfSubGraph(vertex);

    m_vertices_inside_path.push_back(vertex);
    auto& node = m_active_path.emplace(vertex);

    m_graph.ForEachAvailableEdge([&](const Graph::Details::Edge& edge)
    {
        auto [i, j] = edge.GetCurrentSubgraphs(m_graph);
        if (i == vertex || j == vertex)
            node.GetHeap().Insert(edge);
    });
}

template<size_t c>
uint32_t MSTTree<c>::GetTargetSize(uint32_t node_height) const
{
    return m_target_sizes_per_height[node_height];
}

template<size_t c>
uint32_t MSTTree<c>::IndexToHeight(uint32_t index) const
{
    return m_max_height - index -1;
}

void FindMST(Graph::Graph& graph)
{
    MSTTree<2> tree{ graph };
}
}
