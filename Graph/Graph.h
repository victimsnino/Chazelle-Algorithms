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
class Graph
{
public:
    Graph() = default;

    void AddEdge(uint32_t begin, uint32_t end, uint32_t weight);
    void ContractEdge(uint32_t begin, uint32_t end);

    using VertexFunction = std::function<void(uint32_t i, const std::map<uint32_t, uint32_t>& edges)>;
    void ForEachVertex(const VertexFunction& function, bool original = false) const;

    using EdgeFunction = std::function<void(uint32_t i, uint32_t j, uint32_t weight)>;
    void ForEachEdge(const EdgeFunction& function, bool original = false) const;

    [[nodiscard]] size_t GetEdgesCount() const;
    size_t               GetVertexesCount() const { return m_adjacency_matrix.size(); }
    bool IsMstEdge(uint32_t i, uint32_t j) const;
    const std::vector<std::pair<uint32_t, uint32_t>>& GetMST() const { return m_mst; }
private:
    uint32_t GetCurrentVertexAfterContracts(uint32_t vertex) const;
private:
    std::map<uint32_t, std::map<uint32_t, uint32_t>> m_adjacency_matrix{};
    std::map<uint32_t, std::map<uint32_t, uint32_t>> m_original_adjacency_matrix{};
    std::map<uint32_t, uint32_t>                     m_vertex_to_cluster{};
    std::vector<std::pair<uint32_t, uint32_t>>       m_mst{};
};

void ToFile(const Graph& graph, const std::string& graph_name, bool show = false, bool with_mst = false);

void BoruvkaPhase(Graph& graph);

std::pair<uint32_t, uint32_t> BuildPairForEdge(uint32_t i, uint32_t j);
} // namespace Graph
