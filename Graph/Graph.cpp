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
#include <stdexcept>

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

    AddToVertexToSet(i);
    AddToVertexToSet(j);

    m_subset_to_rank.try_emplace(i, 0);
    m_subset_to_rank.try_emplace(j, 0);
}

size_t Graph::GetEdgesCount()
{
    std::map<size_t, std::map<size_t, const Details::Edge*>> matrix{};
    ForValidEdges([&](const Details::Edge& edge, size_t i, size_t j)
    {
        auto  root_i = std::min(i, j);
        auto  root_j = std::max(i, j);
        auto& value  = matrix[root_i][root_j];
        if (!value || value->w > edge.w)
            value = &edge;
    });

   size_t count = 0;
    for(auto& [_, map]: matrix)
        count += map.size();
    return count;
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

        auto itr = m_edges.find(edge_index.value());
        if (itr == m_edges.cend())
            continue;

        auto i = GetRoot(itr->second.i);
        auto j = GetRoot(itr->second.j);

        m_edges.erase(itr);

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

size_t Graph::GetRoot(size_t v)
{
    auto value = GetRootIfExists(v);
    if (!value.has_value())
        throw std::out_of_range("");
    return value.value();
}

std::optional<size_t> Graph::GetRootIfExists(size_t v)
{
    if (v < m_vertex_to_parent.size())
    {
        if (m_vertex_to_parent[v].has_value() && v != m_vertex_to_parent[v])
            m_vertex_to_parent[v] = GetRootIfExists(m_vertex_to_parent[v].value());
        return m_vertex_to_parent[v];
    }
    return {};
}

void Graph::Union(size_t i, size_t j)
{
    auto root_i_opt = GetRootIfExists(i); 
    auto root_j_opt = GetRootIfExists(j);

    if (!root_i_opt || !root_j_opt)
        return;

    auto& root_i = root_i_opt.value();
    auto& root_j = root_j_opt.value();

    // merge smaller tree to larger one by comparing rank
    const auto itr_i = m_subset_to_rank.find(root_i);
    const auto itr_j = m_subset_to_rank.find(root_j);
    if (itr_i->second < itr_j->second)
    {
        m_subset_to_rank.erase(itr_i);
        m_vertex_to_parent[i] = root_j;
    }
    else if (itr_i->second > itr_j->second)
    {
        m_subset_to_rank.erase(itr_j);
        m_vertex_to_parent[j] = root_i;
    }
    // If ranks are same
    else
    {
        m_subset_to_rank.erase(itr_i);
        m_vertex_to_parent[i] = root_j;
        itr_j->second += 1;
    } 
}

void Graph::DisableEdge(size_t index)
{
    m_edges[index].disabled = true;
}

void Graph::ForValidEdges(std::function<void(const Details::Edge&, size_t i, size_t j)> action)
{
    for (auto itr = m_edges.begin(); itr != m_edges.end();)
    {
        if (itr->second.disabled)
        {
            itr = m_edges.erase(itr);
            continue;
        }

        const auto i = GetRoot(itr->second.i);
        const auto j = GetRoot(itr->second.j);

        if (i == j)
        {
            itr = m_edges.erase(itr);
            continue;
        }

        action(itr->second, i, j);
        ++itr;
    }
}

void Graph::AddToVertexToSet(size_t vertex)
{
    if (m_vertex_to_parent.size() <= vertex)
        m_vertex_to_parent.resize(vertex+1);
    m_vertex_to_parent[vertex] = vertex;
}
} // namespace Graph
