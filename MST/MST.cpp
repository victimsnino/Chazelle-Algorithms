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

#include "MST.h"

#include "MSTTreeBuilder.h"
#include "MSTUtils.h"

#include <Common.h>
#include <Graph.h>
#include <spdlog/spdlog.h>

#include <iterator>


namespace MST
{
std::vector<size_t> MSF(Graph::Graph& graph, size_t max_height, size_t recursion_level = 1)
{
    size_t t = FindParamT(graph, max_height <= 2 ? 3 : max_height);
    size_t count = t == 1 ? std::numeric_limits<uint32_t>::max() : c;

    if (recursion_level == 1)
        std::cout << " t is  " << t << std::endl;

    SPDLOG_DEBUG("t is {}", t);

    std::vector<size_t> boruvka_result{};

    while (count > 0 && graph.GetEdgesCount() > 0)
    {
        std::ranges::move(graph.BoruvkaPhase(), std::back_inserter(boruvka_result));
        --count;
    }

    if (graph.GetEdgesCount() == 0)
        return boruvka_result;

    std::list<size_t> vertices = graph.GetVertices();

    auto tree_builder = MSTTreeBuilder(graph.GetEdgesView(), t, max_height);
    auto& tree = tree_builder.GetTree();

    for(auto vert : tree.GetVerticesInside())
        vertices.remove(vert);

    auto& bad_edges = tree_builder.GetBadEdges();

    auto graphs = tree.CreateSubGraphs(bad_edges);

    std::vector<size_t> F = bad_edges;
    for (auto& subgraph : graphs)
        std::ranges::move(MSF(subgraph, max_height, recursion_level +1), std::back_inserter(F));

    std::vector<size_t> edges_to_disable{};
    graph.ForEachAvailableEdge([&](const Graph::Details::Edge& edge)
    {
        if (!Utils::IsRangeContains(F, edge.GetOriginalIndex()))
            edges_to_disable.push_back(edge.GetOriginalIndex());
        else
        {
            if constexpr (s_mst_debug)
                F.erase(std::remove(F.begin(), F.end(), edge.GetOriginalIndex()), F.end());
        }
    });

    for (auto edge : edges_to_disable)
        graph.DisableEdge(edge);

    if constexpr (s_mst_debug)
        if (!F.empty())
            throw std::exception("F is not empty");

    std::ranges::move(MSF(graph, max_height, recursion_level +1), std::back_inserter(boruvka_result));
    return boruvka_result;
}

std::vector<size_t> FindMST(Graph::Graph& graph)
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("Line: %4# [%-35!] %v");

    return MSF(graph, FindMaxHeight(graph, c));
}
} // namespace MST
