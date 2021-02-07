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
    for (const auto& matrix_ptr : {&m_adjacency_matrix, &m_original_adjacency_matrix})
    {
        (*matrix_ptr)[begin][end] = weight;
        (*matrix_ptr)[end][begin] = weight;
    }
}

void Graph::ContractEdge(uint32_t begin, uint32_t end)
{
    auto pair            = BuildPairForEdge(begin, end);
    std::tie(begin, end) = pair;
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

    m_contracted_edges.emplace(std::move(pair));
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

void ToFile(const Graph& graph, const std::string& graph_name, bool show, bool with_mst)
{
#ifndef GRAPHVIZ_DISABLED
    const auto    filename = graph_name + ".dot";
    std::ofstream file_to_out{filename};
    file_to_out << "strict graph {" << std::endl;

    const auto& contracted_edges = graph.GetContractedEdges();

    graph.ForEachEdge([&](uint32_t i, uint32_t j, uint32_t weight)
                      {
                          std::string options = "label="s + std::to_string(weight);
                          if (with_mst && contracted_edges.count(BuildPairForEdge(i, j)))
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
        const auto [j, weight] = *std::min_element(edges.cbegin(),
                                                   edges.cend(),
                                                   [](const std::pair<uint32_t, uint32_t>& first_edge,
                                                      const std::pair<uint32_t, uint32_t>& second_edge)
                                                   {
                                                       return first_edge.second < second_edge.second;
                                                   });
        edges_to_contract.insert(BuildPairForEdge(i, j));
    });
    size_t count = 0;
    for (const auto& [i, j] : edges_to_contract)
    {
        graph.ContractEdge(i, j);
        ToFile(graph, "Boruvka_debug_"s + std::to_string(++count) + "__"+std::to_string(i)+"_"+std::to_string(j), true);
    }
}
} // namespace Graph
