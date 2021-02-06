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

#include <array>
#include <array>
#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace Graph
{
struct Vertex;

struct VertexDoublePtr
{
    VertexDoublePtr(Vertex** ptr)
        : m_ptr(ptr) {}

    operator Vertex*() const { return *m_ptr; }
    Vertex* operator->() const { return *m_ptr; }
private:
    Vertex** m_ptr{};
};

struct Edge
{
    Edge(VertexDoublePtr begin, VertexDoublePtr end, uint32_t weight);

    uint32_t                              GetWeight() const { return m_weight; }
    const std::array<VertexDoublePtr, 2>& GetVertexes() const { return m_vertexes; }
    std::string                           ToFile() const;
private:
    const std::array<VertexDoublePtr, 2> m_vertexes;
    const uint32_t                       m_weight;
};

using EdgePtr = std::shared_ptr<Edge>;

struct Vertex
{
    using LabelType = std::string;
    Vertex(LabelType label);

    LabelType   GetLabel() const { return m_label; }
    const auto& GetEdges() const { return m_edges; }

    void AddEdge(const EdgePtr& edge) { m_edges.insert(edge); }
    void RemoveEdge(const EdgePtr& edge) { m_edges.erase(edge); }
private:
    std::set<EdgePtr, std::function<bool(const EdgePtr&, const EdgePtr&)>> m_edges{
        [](const EdgePtr& left, const EdgePtr& right) -> bool { return left->GetWeight() < right->GetWeight(); }
    };
    const LabelType m_label;
};

class Graph
{
public:
    Graph() = default;

    void AddEdge(const Vertex::LabelType& begin, const Vertex::LabelType& end, uint32_t weight);
    void ToFile(const std::string& graph_name, bool show = false);

    void ContractEdge(const EdgePtr& edge);
    void ForEachVertex(std::function<void(const Vertex&)> function) const;

    size_t GetEdgesCount() const { return m_edges.size(); }
    size_t GetVertexesCount() const { return m_original_vertexes.size(); }
private:
    VertexDoublePtr AddOrReturnVertex(const Vertex::LabelType& label);
    void            ChangeOldVertexToContracted(Vertex* new_vertex, const VertexDoublePtr& vertex);
    void            CreateContractedVertex(const std::array<VertexDoublePtr, 2>& vertexes, const EdgePtr& same_edge);
private:
    std::list<EdgePtr>                   m_edges{};
    std::list<Vertex>                    m_original_vertexes{};
    std::map<Vertex::LabelType, Vertex*> m_label_to_vertexes{};

    std::list<EdgePtr> m_contracted_edges{};
};
} // namespace Graph
