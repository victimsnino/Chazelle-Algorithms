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

#include "Graph.h"

#include <algorithm>
#include <ranges>


namespace Graph
{
Graph::Graph(const std::vector<std::vector<uint32_t>>& adjacency)
{
    for (size_t i = 0; i < adjacency.size(); ++i)
    {
        for (size_t j = 0; j < i; ++j)
            AddEdge(i, j, std::max(adjacency[i][j], adjacency[j][i]));
    }
}

Graph::Graph(const std::vector<std::tuple<size_t, size_t, size_t>>& edges)
{
    for (auto& [i,j,w] : edges)
        AddEdge(i,j,w);
}
void Graph::AddEdge(size_t i, size_t j, size_t w, std::optional<size_t> index)
{
    if (!w)
        return;

    auto cur_index= index.value_or(m_edges.size());
    m_edges.emplace(cur_index, Details::Edge(std::min(i, j), std::max(i,j), w, cur_index));

    m_vertex_to_set[i] = i;
    m_vertex_to_set[j] = j;

    m_subset_to_rank.try_emplace(i, 0);
    m_subset_to_rank.try_emplace(j, 0);
}

size_t Graph::GetEdgesCount() const
{
    return GetValidEdges().size();
}

std::list<size_t> Graph::BoruvkaPhase()
{
    std::map<size_t, std::optional<size_t>> cheapest_edge_for_each_vertex{};
    for(auto& [index, edge] : m_edges)
    {
        auto i = GetRoot(edge.i);
        auto j = GetRoot(edge.j);

        if (i == j)
            continue;

        for (const auto& subgraph : {i,j})
        {
            auto& cheapest_edge = cheapest_edge_for_each_vertex[subgraph];
            if (!cheapest_edge.has_value() || edge.w < m_edges[cheapest_edge.value()].w)
                cheapest_edge.emplace(index);
        }
    }

    std::list<size_t> result{};
    for(auto& [v, edge_index] : cheapest_edge_for_each_vertex)
    {
        if (!edge_index.has_value())
            continue;

        auto& edge = m_edges[edge_index.value()];

        auto i = GetRoot(edge.i);
        auto j = GetRoot(edge.j);

        if (i == j)
            continue;

        Union(i, j);
        result.push_back(edge_index.value());
    }

    return result;
}

std::set<size_t> Graph::GetVertices() const
{
    std::set<size_t> result{};
    for(auto& [v, r] : m_subset_to_rank)
        result.emplace(v);
    return result;
}

size_t Graph::GetRoot(size_t v) const
{
    return m_vertex_to_set.at(v);
}

std::optional<size_t> Graph::GetRootIfExists(size_t v) const
{
    auto itr = m_vertex_to_set.find(v);
    if (itr == m_vertex_to_set.cend())
        return {};
    return itr->second;
}

void Graph::Union(size_t i, size_t j)
{
    const auto root_i_opt = GetRootIfExists(i); 
    const auto root_j_opt = GetRootIfExists(j);

    if (!root_i_opt || !root_j_opt)
        return;
    const auto& root_i = root_i_opt.value();
    const auto& root_j = root_j_opt.value();

    // merge smaller tree to larger one by comparing rank
    if (m_subset_to_rank[root_i] < m_subset_to_rank[root_j])
    {
       for(auto& [_, set] : m_vertex_to_set)
           if (set == root_i)
               set = root_j;
        m_subset_to_rank.erase(root_i);
    }
    else if (m_subset_to_rank[root_i] > m_subset_to_rank[root_j])
    {
       for(auto& [_, set] : m_vertex_to_set)
           if (set == root_j)
               set = root_i;
        m_subset_to_rank.erase(root_j);
    }
    // If ranks are same
    else
    { 
        for(auto& [_, set] : m_vertex_to_set)
           if (set == root_i)
               set = root_j;

        m_subset_to_rank[root_j] += 1;
        m_subset_to_rank.erase(root_i);
    } 
}

void Graph::DisableEdge(size_t index)
{
    m_edges[index].disabled = true;
}

std::list<const Details::Edge*> Graph::GetValidEdges() const
{
    std::map<size_t, std::map<size_t, std::optional<size_t>>> matrix{};
    for (auto& [index,edge] : m_edges)
    {
        if (edge.disabled)
            continue;

        auto i = GetRoot(edge.i);
        auto j = GetRoot(edge.j);

        if (i == j)
            continue;

        auto  root_i = std::min(i, j);
        auto  root_j = std::max(i, j);
        auto& value  = matrix[root_i][root_j];
        if (!value.has_value() || m_edges.at(value.value()).w > edge.w)
            value = index;
    }
    std::list<const Details::Edge*> result{};
    for(auto& [_, map]: matrix)
        for(auto& [_, index] :  map)
            result.push_back(&m_edges.at(index.value()));

    return result;
}
} // namespace Graph
