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

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "MST.h"

#include "MSTTreeBuilder.h"
#include "MSTUtils.h"

#include <Common.h>
#include <Graph.h>
#include <spdlog/spdlog.h>


#include <iterator>


namespace MST
{
std::list<size_t> MSF(Graph::Graph& graph, size_t max_height, size_t recursion_level = 1, size_t t = 0)
{
    SPDLOG_DEBUG("max_height {}", max_height);
    if (!t)
        t = FindParamT(graph, max_height <= 2 ? 3 : max_height);
    size_t count = t <= 1 ? std::numeric_limits<uint32_t>::max() : c;

    if (recursion_level == 1)
        std::cout << " t is  " << t << std::endl;

    SPDLOG_DEBUG("t is {} Recursion {}", t, recursion_level);

    bool no_changes = false;
    std::list<size_t> boruvka_result = graph.BoruvkaPhase(count, &no_changes);
    if (no_changes)
        return boruvka_result;


    std::set<size_t> vertices = graph.GetVertices();
    std::list<size_t> bad_edges ={};
    std::list<Graph::Graph> graphs{};
    while (!vertices.empty())
    {
        auto  tree_builder = MSTTreeBuilder(graph, t, max_height, *vertices.begin());
        auto& tree         = tree_builder.GetTree();

        for (const auto& vert : tree.GetVerticesInside())
            vertices.erase(vert);

        auto cur_bad_edges = tree.GetBadEdges();
        graphs.splice(graphs.end(), tree.CreateSubGraphs(cur_bad_edges));
        std::ranges::move(cur_bad_edges, std::inserter(bad_edges, bad_edges.end()));
    }

    //SPDLOG_INFO("BAD EDGES COUNT {}", bad_edges.size());

    //for(auto& edge : bad_edges)
    //    SPDLOG_DEBUG("{}", edge);

    std::list<size_t> F = bad_edges;
    for (auto& subgraph : graphs)
        F.splice(F.end(), MSF(subgraph, max_height, recursion_level + 1, t > 1 ? t-1 : t));


    Graph::Graph new_graph{};
    for (const auto& edge : F)
    {
        auto& orig_edge = graph.GetEdge(edge);
        auto i = graph.GetRoot(orig_edge.i);
        auto j = graph.GetRoot(orig_edge.j);
        new_graph.AddEdge(i, j, orig_edge.w, edge);
    }

    boruvka_result.splice(boruvka_result.end(), MSF(new_graph, max_height, recursion_level+1, t));
    return boruvka_result;
}

std::list<size_t> FindMST(Graph::Graph& graph)
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("Line: %4# [%-35!] %v");

    return MSF(graph, FindMaxHeight(graph, c));
}
} // namespace MST
