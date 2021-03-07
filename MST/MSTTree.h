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

#pragma once

#include <SoftHeapCpp.h>
#include <Graph.h>

#include <cstdint>
#include <map>
#include <set>
#include <set>
#include <set>
#include <stack>
#include <vector>


using Edge = Graph::Details::Edge;
using Cluster = std::set<std::reference_wrapper<Edge>, decltype([](const std::reference_wrapper<Edge>& left,
                                                                   const std::reference_wrapper<Edge>&
                                                                   right)
{
    return left.get() < right.get();
})>;

namespace MST
{
class MSTTree
{
public:
    MSTTree(Graph::Graph& graph, size_t c);

private:
    using SoftHeap = SoftHeapCpp<Edge>;

    class TreeNode
    {
    public:
        TreeNode(size_t r)
            : m_heap{r} {}

        TreeNode(size_t vertex, size_t r)
            : m_heap{r}
        {
            AddVertex(vertex);
        }

        void  AddVertex(size_t vertex) { m_vertices.push_back(vertex); }
        auto& GetHeap() { return m_heap; }

        size_t                     GetCountOfVertices() const { return m_vertices.size(); }
        const std::vector<size_t>& GetVertices() const { return m_vertices; }

    private:
        std::vector<size_t> m_vertices{};
        SoftHeap            m_heap;
    };

private:
    // Step 1,2: Boruvka
    bool BoruvkaPhase(size_t c) const;

    // Preparation for step 3
    void InitiateTree();

    // Step 3: Building tree T
    void BuildTree();

private:
    bool Extension();
    void Retraction();

    void CreateClusters(size_t k, std::list<Edge>&& valid_items);

    void CreateOneVertexNode(size_t vertex);
    void ContractNode(const TreeNode& last_subgraph);

    void InsertEdgeToLastNodeHeap(Edge& edge);
    void InsertToHeapForEdge(::Graph::Details::Edge& edge,
                             const Cluster&          his_cluster,
                             size_t                  k);

    uint32_t GetTargetSize(uint32_t node_height) const;
    uint32_t IndexToHeight(uint32_t index) const;
    SoftHeap& GetHeap(size_t i, size_t j);

private:
    Graph::Graph&                                m_graph;
    const uint32_t                               m_max_height;
    const uint32_t                               m_t;
    const size_t                                 m_r;
    std::vector<uint32_t>                        m_target_sizes_per_height{};
    std::stack<TreeNode>                         m_active_path{};
    std::vector<size_t>                          m_vertices_inside_path{};
    std::map<size_t, std::map<size_t, SoftHeap>> m_heaps{};
};
}
