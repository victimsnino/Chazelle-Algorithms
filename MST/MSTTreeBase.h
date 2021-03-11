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
#include "MSTSoftHeapDecorator.h"

#include <Graph.h>

#include <stack>
#include <vector>

namespace MST::Details
{
class TreeNode
{
public:
    using Heap = MSTSoftHeapDecorator;

    TreeNode(size_t r, size_t label)
        : m_label{label}
        , m_heap{r, label}
    {
        BuildCrossHeaps(r);
    }

    TreeNode(size_t vertex, size_t r, size_t label)
        : m_label{label}
        , m_heap{r, label}
    {
        BuildCrossHeaps(r);
        AddVertex(vertex);
    }

    void              AddVertex(size_t vertex) { m_vertices.push_back(vertex); }

    Heap& GetCrossHeapFrom(size_t i)
    {
        assert(i < m_cross_heaps.size());
        return m_cross_heaps[i];
    }

    Heap&                      GetHeap() { return m_heap; }
    std::vector<Heap>&         GetCrossHeaps() { return m_cross_heaps; }
    const std::vector<size_t>& GetVertices() const { return m_vertices; }
    size_t                     GetLabel() const { return m_label; }
private:
    void BuildCrossHeaps(size_t r);

private:
    const size_t        m_label;
    std::vector<size_t> m_vertices{};

    Heap              m_heap;
    std::vector<Heap> m_cross_heaps;
};

class MSTTreeBase
{
public:
    using SoftHeap = TreeNode::Heap;
    MSTTreeBase(Graph::Graph& graph, size_t c);

    TreeNode ContractLastNode();

    /*======================================= GETTERS ===================================== */
    Graph::Graph& GetGraph() const { return m_graph; }
    TreeNode&     GetLastNode() { return m_active_path.top(); }

    bool                       IsCanRetraction() const;
    size_t                     UpdateNodeIndex(size_t i) const { return m_graph.FindRootOfSubGraph(i); }
    const std::vector<size_t>& GetVerticesInsidePath() const { return m_vertices_inside_path; }
private:
    /* ===================================== MODIFIERS =====================================*/
    void CreateOneVertexNode(size_t vertex);
    void ContractNode(const TreeNode& node);
    void UpdateActivePathIndexes();

    /*======================================= GETTERS ===================================== */
    size_t GetMaxHeight() const { return m_target_sizes_per_height.size() - 1; }
    size_t GetTargetSize(size_t node_index) const { return m_target_sizes_per_height[GetMaxHeight() - node_index]; }

private:
    Graph::Graph&             m_graph;
    const size_t              m_r;
    const std::vector<size_t> m_target_sizes_per_height;

    std::stack<TreeNode> m_active_path;
    std::vector<size_t>  m_vertices_inside_path{};
};
}
