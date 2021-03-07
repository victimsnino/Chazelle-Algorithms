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

#include "GraphDetails.h"

#include <array>
#include <array>
#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace Graph
{
class Graph
{
public:
    Graph(const std::vector<std::vector<uint32_t>>& adjacency);

    Graph() = default;

    void BoruvkaPhase();
    void ContractEdge(size_t edge_index);
    void DisableEdge(size_t edge_index);

    size_t GetVertexesCount()const;
    size_t GetEdgesCount() const;
    std::vector<std::array<size_t, 2>> GetMST() const;

    size_t FindRootOfSubGraph(size_t i);

    void ForEachAvailableEdge(const std::function<void(Details::Edge& edge)>& func);
    void ForEachAvailableEdge(const std::function<void(const Details::Edge& edge)>& func) const;

private:
    void   AddEdge(size_t begin, size_t end, uint32_t weight);
    void   RemoveMultipleEdgesForVertex(size_t vertex_id);

private:
    Details::EdgesView                     m_edges_view{};
    std::vector<Details::MemberOfSubGraph> m_subgraphs{}; // aka vertexes

    friend void ToFile(Graph& graph, const std::string& graph_name, bool show, bool with_mst);
};

void ToFile(Graph& graph, const std::string& graph_name, bool show = false, bool with_mst = false);
} // namespace Graph
