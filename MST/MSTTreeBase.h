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
    TreeNode(size_t r, size_t label)
        : m_heap{r, label} {}

    TreeNode(size_t vertex, size_t r, size_t label)
        : m_heap{r, label}
    {
        AddVertex(vertex);
    }

    void                  AddVertex(size_t vertex) { m_vertices.push_back(vertex); }
    MSTSoftHeapDecorator& GetHeap() { return m_heap; }

    const std::vector<size_t>& GetVertices() const { return m_vertices; }

private:
    std::vector<size_t>  m_vertices{};
    MSTSoftHeapDecorator m_heap;
};

class MSTTreeBase
{
public:
    using SoftHeap = MSTSoftHeapDecorator;

    MSTTreeBase(Graph::Graph& graph, size_t c);

    SoftHeap::ExtractedItems ContractLastAddToNextAndExtractEdgesFromHeap();
    void                     MeldHeapsFromTo(std::array<size_t, 2> from, std::array<size_t, 2> to);

    /*======================================= GETTERS ===================================== */
    bool                       IsCanRetraction() const;
    Graph::Graph&              GetGraph() const { return m_graph; }
    size_t                     GetIndexOfLastInPath() const { return m_active_path.size() - 1; }
    const std::vector<size_t>& GetVerticesInsidePath() const { return m_vertices_inside_path; }

    SoftHeap& GetLastNodeHeap();
    SoftHeap& GetHeap(size_t i, size_t j);

private:
    /* ===================================== MODIFIERS =====================================*/
    void CreateOneVertexNode(size_t vertex);
    void ContractNode(const TreeNode& node);
    void UpdateActivePathIndexes();

    /*======================================= GETTERS ===================================== */
    size_t GetMaxHeight() const { return m_target_sizes_per_height.size() - 1; }
    size_t UpdateNodeIndex(size_t i) const { return m_graph.FindRootOfSubGraph(i); }
    size_t GetTargetSize(size_t node_index) const;
private:
    Graph::Graph&             m_graph;
    const size_t              m_r;
    const std::vector<size_t> m_target_sizes_per_height;

    std::stack<TreeNode> m_active_path;
    std::vector<size_t>  m_vertices_inside_path{};

    std::map<size_t, std::map<size_t, SoftHeap>> m_cross_heaps{};
};
}
