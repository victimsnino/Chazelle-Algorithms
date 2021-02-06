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
#include <array>
#include <array>
#include <fstream>
#include <utility>

using namespace std::string_literals;

namespace Graph
{
Edge::Edge(VertexDoublePtr begin, VertexDoublePtr end, uint32_t weight)
    : m_vertexes{begin, end}
    , m_weight{weight} { }

std::string Edge::ToFile() const
{
    return "v"s+m_vertexes[0]->GetLabel()
            + " -- v"
            + m_vertexes[1]->GetLabel()
            + " [label="
            + std::to_string(m_weight)
            + "]";
}

Vertex::Vertex(LabelType label)
    : m_label(std::move(label)) { }

void Graph::AddEdge(const Vertex::LabelType& begin, const Vertex::LabelType& end, uint32_t weight)
{
    auto       vertex_1 = AddOrReturnVertex(begin);
    auto       vertex_2 = AddOrReturnVertex(end);
    const auto edge     = m_edges.emplace_back(std::make_shared<Edge>(vertex_1, vertex_2, weight));
    vertex_1->AddEdge(edge);
    vertex_2->AddEdge(edge);
}

void Graph::ToFile(const std::string& graph_name, bool show)
{
#ifndef GRAPHVIZ_DISABLED
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
#endif
}

void Graph::ContractEdge(const EdgePtr& edge)
{
    m_contracted_edges.splice(m_contracted_edges.begin(),
                              m_edges,
                              std::find(m_edges.begin(), m_edges.end(), edge));

    CreateContractedVertex(edge);
}

void Graph::ForEachVertex(std::function<void(const Vertex&)> function) const
{
    std::for_each(m_original_vertexes.cbegin(),
                  m_original_vertexes.cend(),
                  std::move(function));
}

VertexDoublePtr Graph::AddOrReturnVertex(const Vertex::LabelType& label)
{
    auto itr = m_label_to_vertexes.find(label);

    if (itr == m_label_to_vertexes.cend())
        itr = m_label_to_vertexes.emplace(label, &m_original_vertexes.emplace_back(label)).first;

    return &itr->second;
}

void Graph::ChangeOldVertexToContracted(Vertex* new_vertex, const VertexDoublePtr& vertex)
{
    std::vector<decltype(m_label_to_vertexes)::iterator> iterators_to_edit{};
    for (auto itr = m_label_to_vertexes.begin(); itr != m_label_to_vertexes.end(); ++itr)
        if (itr->second == vertex)
            iterators_to_edit.push_back(itr);

    m_original_vertexes.remove_if([&](const Vertex& to_check) { return &to_check == vertex; });

    for (auto& itr : iterators_to_edit)
        itr->second = new_vertex;
}

void Graph::CreateContractedVertex(const EdgePtr& same_edge)
{
    auto vertexes = same_edge->GetVertexes();
    Vertex* new_vertex = &m_original_vertexes.emplace_back(vertexes[0]->GetLabel() + "_" + vertexes[1]->GetLabel());

    for (const auto& vertex : vertexes)
    {
        for (const auto& edge_to_move : vertex->GetEdges())
        {
            if (edge_to_move->GetVertexes() == vertexes)
            {
                m_edges.remove(edge_to_move);
                continue;
            }
            if (auto itr = std::find_if(new_vertex->GetEdges().begin(),
                new_vertex->GetEdges().end(),
                [&](const EdgePtr& edge_to_check)
                {
                    return edge_to_check->GetVertexes() == edge_to_move->GetVertexes();
                }); itr != new_vertex->GetEdges().end())
            {}

                new_vertex->AddEdge(edge_to_move)
        }

        ChangeOldVertexToContracted(new_vertex, vertex);
    }
}
} // namespace Graph
