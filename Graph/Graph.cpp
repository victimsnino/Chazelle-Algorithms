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
#include <fstream>
#include <optional>
#include <utility>

using namespace std::string_literals;

namespace Graph
{
namespace Details
{
    Edge::Edge(size_t i, size_t j, uint32_t weight)
        : m_i(i)
        , m_j(j)
        , m_weight(weight) { }

    MemberOfSubGraph::MemberOfSubGraph(size_t parent, size_t rank)
        : m_parent(parent)
        , m_original_vertex(parent)
        , m_rank(rank) {}
} // namespace Details

void Graph::AddEdge(size_t begin, size_t end, uint32_t weight)
{
    if (begin == end)
        return;

    m_edges.emplace_back(std::min(begin, end), std::max(begin, end), weight);
}

void Graph::ContractEdge(size_t edge_index)
{
    const auto [i, j] = m_edges[edge_index].GetVertexes();

    auto& root_subgraph_1 = m_subgraphs[FindRootOfSubGraph(i)];
    auto& root_subgraph_2 = m_subgraphs[FindRootOfSubGraph(j)];

    if (root_subgraph_1 == root_subgraph_2)
        return;

    if (root_subgraph_1 > root_subgraph_2)
        root_subgraph_2.SetParent(root_subgraph_1.GetParent());
    else
        root_subgraph_1.SetParent(root_subgraph_2.GetParent());

    m_edges[edge_index].SetIsContracted();
}

size_t Graph::FindRootOfSubGraph(size_t i)
{
    auto member_of_subgraph = m_subgraphs[i];

    // it is not a root... update root
    if (!member_of_subgraph.IsRoot())
        member_of_subgraph.SetParent(FindRootOfSubGraph(member_of_subgraph.GetParent()));

    return member_of_subgraph.GetParent();
}

void Graph::BoruvkaPhase()
{
    std::vector<std::optional<size_t>> cheapest_edge_for_each_vertex(m_subgraphs.size(), std::nullopt);
    for (const auto&& [edge_index, edge] : Utils::enumerate(m_edges))
    {
        if (edge.IsContracted())
            continue;

        const auto [i, j]     = edge.GetVertexes();
        const auto subgraph_1 = FindRootOfSubGraph(i);
        const auto subgraph_2 = FindRootOfSubGraph(j);

        if (subgraph_1 == subgraph_2)
            continue;

        for (const auto& subgraph : {subgraph_1, subgraph_2})
        {
            auto& cheapest_edge = cheapest_edge_for_each_vertex[subgraph];
            if (!cheapest_edge.has_value() || m_edges[cheapest_edge.value()] > edge)
                cheapest_edge.emplace(edge_index);
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
    // to-do
    return std::count_if(m_edges.cbegin(),
                         m_edges.cend(),
                         [](const Details::Edge& edge) { return !edge.IsContracted(); });
}

std::vector<std::tuple<size_t, size_t>> Graph::GetMST() const
{
    std::vector<std::tuple<size_t, size_t>> result{};
    for (const auto& edge : m_edges)
        if (edge.IsContracted())
            result.emplace_back(edge.GetVertexes());
    return result;
}

void ToFile(const Graph& graph, const std::string& graph_name, bool show, bool with_mst)
{
#ifndef GRAPHVIZ_DISABLED
    //const auto    filename = graph_name + ".dot";
    //std::ofstream file_to_out{filename};
    //file_to_out << "strict graph {" << std::endl;

    //graph.ForEachEdge([&](uint32_t i, uint32_t j, uint32_t weight)
    //                  {
    //                      std::string options = "label="s + std::to_string(weight);
    //                      if (with_mst && graph.IsMstEdge(i, j))
    //                          options += ", color=red, penwidth=3";
    //                      file_to_out << i << " -- " << j << " [" << options << "]" << std::endl;
    //                  },
    //                  with_mst);

    //file_to_out << "}";
    //file_to_out.close();

    //if (!show)
    //    return;

    //const auto image_file_name = graph_name + ".png";
    //system(("dot "s + filename + " -Tpng -o " + image_file_name).c_str());
    //system(image_file_name.c_str());
#endif
}
} // namespace Graph
