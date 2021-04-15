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
    Graph(const std::vector<std::tuple<size_t, size_t, size_t>>& edges); // i,j, w


    Graph(Graph&& other) noexcept
        : m_edges_view{std::move(other.m_edges_view)}
        , m_subgraphs{std::move(other.m_subgraphs)} {}

    Graph& operator=(Graph&& other) noexcept
    {
        if (this == &other)
            return *this;
        m_edges_view = std::move(other.m_edges_view);
        m_subgraphs  = std::move(other.m_subgraphs);
        return *this;
    }

    Graph() = default;

    void AddEdge(size_t begin, size_t end, uint32_t weight, std::optional<size_t> original_index = {});
    void UnionVertices(size_t i, size_t j);

    std::vector<size_t> BoruvkaPhase();

    void                ContractEdge(size_t edge_index);
    void                DisableEdge(size_t edge_index);

    std::list<size_t>    GetVertices() const;
    size_t               GetVerticesCount() const;
    size_t               GetEdgesCount() const;
    const Details::Edge& GetEdge(size_t index) const { return m_edges_view[index]; }

    size_t FindRootOfSubGraph(size_t i);

    void ForEachAvailableEdge(const std::function<void(Details::Edge& edge)>& func);
    void ForEachAvailableEdge(const std::function<void(const Details::Edge& edge)>& func) const;

    Details::EdgesView& GetEdgesView() {return m_edges_view;}
private:
    void RemoveMultipleEdgesForVertex(size_t vertex_id);
    std::shared_ptr<Details::MemberOfSubGraph> GetOrCreateSubgraph(size_t index);
    std::shared_ptr<Details::MemberOfSubGraph> GetSubgraphIfExists(size_t index);

private:
    Details::EdgesView                                      m_edges_view{};
    std::vector<std::shared_ptr<Details::MemberOfSubGraph>> m_subgraphs{}; // aka vertices

    friend void ToFile(Graph& graph, const std::string& graph_name, bool show, bool with_mst);
};

void ToFile(Graph& graph, const std::string& graph_name, bool show = false, bool with_mst = false);
} // namespace Graph
