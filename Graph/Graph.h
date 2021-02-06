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

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Graph
{
struct Vertex;

struct Edge
{
    using Ptr = std::shared_ptr<Edge>;

    Edge(Vertex* begin, Vertex* end, uint32_t weight);

    uint32_t GetWeight() const { return m_weight; }

    std::string ToFile() const;
private:
    const Vertex*  m_begin;
    const Vertex*  m_end;
    const uint32_t m_weight;
};

struct Vertex
{
    Vertex(uint32_t label);

    uint32_t GetLabel() const{return m_label;}

    void AddEdge(const Edge::Ptr& edge) { m_edges.insert(edge); }
private:
    std::set<Edge::Ptr, std::function<bool(const Edge::Ptr&, const Edge::Ptr&)>> m_edges{
        [](const Edge::Ptr& left, const Edge::Ptr& right) -> bool { return left->GetWeight() < right->GetWeight(); }
    };
    uint32_t m_label;
};

class Graph
{
public:
    Graph() = default;

    void AddEdge(uint32_t begin, uint32_t end, uint32_t weight);
    void ToFile(const std::string& graph_name, bool show = false);

    size_t GetEdgesCount() const { return m_edges.size(); }
    size_t GetVertexesCount() const { return m_vertexes.size(); }
private:
    std::vector<Edge::Ptr>     m_edges{};
    std::map<uint32_t, Vertex> m_vertexes{};
};
} // namespace Graph
