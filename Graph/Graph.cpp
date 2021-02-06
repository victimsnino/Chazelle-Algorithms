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

#include <fstream>

using namespace std::string_literals;

namespace Graph
{
Edge::Edge(Vertex* begin, Vertex* end, uint32_t weight)
    : m_begin(begin)
    , m_end(end)
    , m_weight(weight) { }

std::string Edge::ToFile() const
{
    return std::to_string(m_begin->GetLabel())
            + " -- "
            + std::to_string(m_end->GetLabel())
            + " [label="s
            + std::to_string(m_weight)
            + "]";
}

Vertex::Vertex(uint32_t label)
    : m_label(label) { }

void Graph::AddEdge(uint32_t begin, uint32_t end, uint32_t weight)
{
    auto*      vertex_1 = &m_vertexes.try_emplace(begin, begin).first->second;
    auto*      vertex_2 = &m_vertexes.try_emplace(end, end).first->second;
    const auto edge     = m_edges.emplace_back(std::make_shared<Edge>(vertex_1, vertex_2, weight));
    vertex_1->AddEdge(edge);
    vertex_2->AddEdge(edge);
}

void Graph::ToFile(const std::string& graph_name, bool show)
{
    const auto    filename = graph_name + ".dot";
    std::ofstream file_to_out{filename};
    file_to_out << "strict graph {" << std::endl;
    for (const auto& edge : m_edges)
    {
        file_to_out << edge->ToFile() << std::endl;
    }
    file_to_out << "}";
    file_to_out.close();

    if (!show)
        return;

    const auto image_file_name = graph_name + ".png";
    system(("dot "s + filename + " -Tpng -o " + image_file_name).c_str());
    system(image_file_name.c_str());
}
} // namespace Graph
