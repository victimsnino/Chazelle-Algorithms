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
    m_adjacency_matrix[begin][end] = weight;
    m_adjacency_matrix[end][begin] = weight;
}

void Graph::ToFile(const std::string& graph_name, bool show)
{
#ifndef GRAPHVIZ_DISABLED
    const auto    filename = graph_name + ".dot";
    std::ofstream file_to_out{filename};
    file_to_out << "strict graph {" << std::endl;
    for (const auto& [i, edges] : m_adjacency_matrix)
    {
        for (const auto& [j, weight] : edges)
        {
            file_to_out << i << " -- " << j << " [label=" << weight << "]" << std::endl;
        }
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

void Graph::ContractEdge(uint32_t begin, uint32_t end)
{
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
} // namespace Graph
