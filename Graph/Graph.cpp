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
#include <fstream>
#include <utility>

using namespace std::string_literals;

namespace Graph
{
void Graph::AddEdge(uint32_t begin, uint32_t end, uint32_t weight)
{
    if (begin == end)
        return;

    for (const auto& matrix_ptr : {&m_adjacency_matrix, &m_original_adjacency_matrix})
    {
        if (auto     itr        = (*matrix_ptr).find(begin); itr != (*matrix_ptr).cend())
            if (auto weight_itr = itr->second.find(end); weight_itr != itr->second.cend())
                weight = std::min(weight, weight_itr->second);

        (*matrix_ptr)[begin][end] = weight;
        (*matrix_ptr)[end][begin] = weight;
    }
}

void Graph::ContractEdge(uint32_t original_begin, uint32_t original_end)
{
    uint32_t begin = GetCurrentVertexAfterContracts(original_begin);
    uint32_t end = GetCurrentVertexAfterContracts(original_end);

    if (begin == end)
        return;

    if (begin > end)
    {
        std::swap(begin, end);
        std::swap(original_begin, original_end);
    }

    for (const auto& [j, weight] : m_adjacency_matrix[end])
    {
        if (j == begin)
            continue;

        const auto new_weight = m_adjacency_matrix[begin][j] > 0 ?
                                    std::min(weight, m_adjacency_matrix[begin][j]) :
                                    weight;
        m_adjacency_matrix[begin][j] = new_weight;
        m_adjacency_matrix[j][begin] = new_weight;
    }

    m_adjacency_matrix.erase(end);

    for (auto& [i, edges] : m_adjacency_matrix)
    {
        edges.erase(end);
    }

    m_vertex_to_cluster[end] = begin;
    m_mst.emplace_back(BuildPairForEdge(original_begin, original_end));
}

void Graph::ForEachVertex(const VertexFunction& function, bool original) const
{
    for (const auto& [i, edges] : original ? m_original_adjacency_matrix : m_adjacency_matrix)
    {
        function(i, edges);
    }
}

void Graph::ForEachEdge(const EdgeFunction& function, bool original) const
{
    ForEachVertex([&](uint32_t i, const std::map<uint32_t, uint32_t>& edges)
                  {
                      for (const auto& [j, weight] : edges)
                      {
                          if (j >= i)
                              function(i, j, weight);
                      }
                  },
                  original);
}

size_t Graph::GetEdgesCount() const
{
    size_t count = {};
    for (const auto& [i, edges] : m_adjacency_matrix)
    {
        count += std::count_if(edges.cbegin(), edges.cend(), [&](const auto& pair) { return pair.first <= i; });
    }
    return count;
}

bool Graph::IsMstEdge(uint32_t i, uint32_t j) const
{
    return std::find_if(m_mst.cbegin(),
                        m_mst.cend(),
                        [&](const auto& pair)
                        {
                            return i == pair.first && j == pair.second || i == pair.second && j == pair.first;
                        })
            != m_mst.cend();
}

uint32_t Graph::GetCurrentVertexAfterContracts(uint32_t vertex) const
{
    auto itr = m_vertex_to_cluster.find(vertex);
    while (itr != m_vertex_to_cluster.cend())
    {
        vertex = itr->second;
        itr = m_vertex_to_cluster.find(vertex);
    }
    return vertex;
}

void ToFile(const Graph& graph, const std::string& graph_name, bool show, bool with_mst)
{
#ifndef GRAPHVIZ_DISABLED
    const auto    filename = graph_name + ".dot";
    std::ofstream file_to_out{filename};
    file_to_out << "strict graph {" << std::endl;

    graph.ForEachEdge([&](uint32_t i, uint32_t j, uint32_t weight)
                      {
                          std::string options = "label="s + std::to_string(weight);
                          if (with_mst && graph.IsMstEdge(i, j))
                              options += ", color=red, penwidth=3";
                          file_to_out << i << " -- " << j << " [" << options << "]" << std::endl;
                      },
                      with_mst);

    file_to_out << "}";
    file_to_out.close();

    if (!show)
        return;

    const auto image_file_name = graph_name + ".png";
    system(("dot "s + filename + " -Tpng -o " + image_file_name).c_str());
    system(image_file_name.c_str());
#endif
}

void BoruvkaPhase(Graph& graph)
{
    std::set<std::pair<uint32_t, uint32_t>> edges_to_contract{};
    graph.ForEachVertex([&](uint32_t        i, const std::map<uint32_t, uint32_t>& edges)
    {
        if (edges.empty())
            return;

        const auto [j, weight] = *std::min_element(edges.cbegin(),
                                                   edges.cend(),
                                                   [](const std::pair<uint32_t, uint32_t>& first_edge,
                                                      const std::pair<uint32_t, uint32_t>& second_edge)
                                                   {
                                                       return first_edge.second < second_edge.second;
                                                   });
        edges_to_contract.insert(BuildPairForEdge(i, j));
    });

    for (const auto& [i, j] : edges_to_contract)
    {
        graph.ContractEdge(i, j);
    }
}

std::pair<uint32_t, uint32_t> BuildPairForEdge(uint32_t i, uint32_t j)
{
    return std::make_pair(std::min(i, j), std::max(i, j));
}
} // namespace Graph
