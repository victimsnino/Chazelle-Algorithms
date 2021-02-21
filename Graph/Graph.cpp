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

#include <Common.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <numeric>
#include <optional>
#include <utility>

using namespace std::string_literals;

namespace Graph
{
Graph::Graph(const std::vector<std::vector<uint32_t>>& adjacency)
{
    m_subgraphs.reserve(adjacency.size());
    for (size_t i = 0; i < adjacency.size(); ++i)
    {
        assert(adjacency.size() == adjacency[i].size());
        m_subgraphs.emplace_back(i);

        for (size_t j = 0; j < i; ++j)
            AddEdge(i, j, std::max(adjacency[i][j], adjacency[j][i]));
    }
}

void Graph::AddEdge(size_t begin, size_t end, uint32_t weight)
{
    if (begin == end || !weight)
        return;

    m_edges_view.AddEdge(std::min(begin, end), std::max(begin, end), weight);
}

void Graph::RemoveMultipleEdgesForVertex(size_t vertex_id)
{
    std::map<size_t, const Details::Edge*> cheapest_out_edges{};
    std::vector<size_t>                    edges_to_disable{};
    for (const auto& edge : m_edges_view)
    {
        auto [i, j] = edge.GetCurrentSubgraphs(*this);

        if (i != vertex_id && j != vertex_id)
            continue;

        auto  out_index = i == vertex_id ? j : i;
        auto& ptr       = cheapest_out_edges[out_index];
        if (!ptr)
        {
            ptr = &edge;
            continue;
        }
        edges_to_disable.push_back(*ptr < edge ? edge.GetIndex() : ptr->GetIndex());
        ptr = edge < *ptr ? &edge : ptr;
    }

    for (auto edge_to_disable : edges_to_disable)
        m_edges_view.DisableEdge(edge_to_disable);
}

void Graph::ContractEdge(size_t edge_index)
{
    const auto [i, j] = m_edges_view[edge_index].GetCurrentSubgraphs(*this);

    auto& root_subgraph_1 = m_subgraphs[i];
    auto& root_subgraph_2 = m_subgraphs[j];

    if (root_subgraph_1 == root_subgraph_2)
        return;

    if (root_subgraph_1 < root_subgraph_2)
        root_subgraph_1.SetParent(root_subgraph_2.GetParent());
    else
        root_subgraph_2.SetParent(root_subgraph_1.GetParent());

    m_edges_view.ContractEdge(edge_index);

    RemoveMultipleEdgesForVertex(root_subgraph_1.GetParent());
}

size_t Graph::FindRootOfSubGraph(size_t i)
{
    auto member_of_subgraph = m_subgraphs[i];

    // it is not a root... update root
    if (!member_of_subgraph.IsRoot())
        member_of_subgraph.SetParent(FindRootOfSubGraph(member_of_subgraph.GetParent()));

    return member_of_subgraph.GetParent();
}

void Graph::ForEachAvailableEdge(const std::function<void(const Details::Edge& edge)>& func) const
{
    std::for_each(m_edges_view.begin(), m_edges_view.end(), func);
}

void Graph::BoruvkaPhase()
{
    std::vector<std::optional<size_t>> cheapest_edge_for_each_vertex(m_subgraphs.size(), std::nullopt);
    for (const auto& edge : m_edges_view)
    {
        const auto [subgraph_1, subgraph_2]     = edge.GetOriginalVertexes();

        if (subgraph_1 == subgraph_2)
            continue;

        for (const auto& subgraph : {subgraph_1, subgraph_2})
        {
            auto& cheapest_edge = cheapest_edge_for_each_vertex[subgraph];
            if (!cheapest_edge.has_value() || edge < m_edges_view[cheapest_edge.value()])
                cheapest_edge.emplace(edge.GetIndex());
        }
    }

    for (const auto& opt_cheapest : cheapest_edge_for_each_vertex)
    {
        if (!opt_cheapest.has_value())
            continue;

        ContractEdge(opt_cheapest.value());
    }
}

size_t Graph::GetVertexesCount() const
{
    return std::count_if(m_subgraphs.cbegin(),
                         m_subgraphs.cend(),
                         [](const Details::MemberOfSubGraph& member) { return member.IsRoot(); });
}

size_t Graph::GetEdgesCount() const
{
    return std::count_if(m_edges_view.begin(),
                         m_edges_view.end(),
                         [](const Details::Edge& edge) { return !edge.IsContracted(); });
}

std::vector<std::tuple<size_t, size_t>> Graph::GetMST() const
{
    std::vector<std::tuple<size_t, size_t>> result{};
    for (const auto& edge : m_edges_view.Original())
        if (edge.IsContracted())
            result.emplace_back(edge.GetOriginalVertexes());
    return result;
}

void ToFile(Graph& graph, const std::string& graph_name, bool show, bool with_mst)
{
#ifndef GRAPHVIZ_DISABLED
    const auto    filename = graph_name + ".dot";
    std::ofstream file_to_out{filename};
    file_to_out << "strict graph {" << std::endl;

    for (const auto& edge : graph.m_edges_view.Original())
    {
        std::string options = "label="s + std::to_string(edge.GetWeight());
        auto [i, j] = edge.GetOriginalVertexes();

        if (with_mst)
        {
            if (edge.IsContracted())
                options += ", color=red, penwidth=3";
        }
        else
        {
            if (edge.IsDisabled())
                continue;
            i = graph.FindRootOfSubGraph(i);
            j = graph.FindRootOfSubGraph(j);
        }
        
        file_to_out << i << " -- " << j << " [" << options << "]" << std::endl;
    }

    file_to_out << "}";
    file_to_out.close();

    if (!show)
        return;

    const auto image_file_name = graph_name + ".png";
    system(("dot "s + filename + " -Tpng -o " + image_file_name).c_str());
    system(image_file_name.c_str());
#endif
}
} // namespace Graph
