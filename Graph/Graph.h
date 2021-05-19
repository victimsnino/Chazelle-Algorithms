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

#include <list>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <vector>

namespace Graph
{
class Graph
{
public:
    Graph() = default;
    Graph(const std::vector<std::vector<uint32_t>>& adjacency);
    Graph(const std::vector<std::tuple<size_t, size_t, size_t>>& edges);

    void AddEdge(size_t i, size_t j, size_t w, std::optional<size_t> index = std::nullopt);
    void Union(size_t i, size_t j);
    void DisableEdge(size_t index);

    size_t GetEdgesCount() const;
    size_t GetVerticesCount() const { return m_subset_to_rank.size(); }

    std::list<size_t>     BoruvkaPhase();
    Details::Edge&        GetEdge(size_t index) { return m_edges[index]; }
    std::set<size_t>      GetVertices() const;
    size_t                GetRoot(size_t v) const;
    std::optional<size_t> GetRootIfExists(size_t v) const;

    std::list<const Details::Edge*> GetValidEdges() const;

private:
    std::map<size_t, Details::Edge> m_edges{};
    std::map<size_t, size_t>        m_vertex_to_set{};
    std::map<size_t, size_t>        m_subset_to_rank{};
};
} // namespace Graph
