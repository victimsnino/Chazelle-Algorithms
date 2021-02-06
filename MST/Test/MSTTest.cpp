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

#include <Graph.h>

#include <gtest/gtest.h>

#include <array>

constexpr bool s_show_graphs = true;

TEST(MST, Init)
{
    constexpr size_t                                      nodes_count = 7;
    std::array<std::array<int, nodes_count>, nodes_count> adjacency_matrix{
        std::array{0, 1, 0, 3, 0, 0, 0},
        std::array{0, 0, 2, 0, 4, 0, 0},
        std::array{0, 0, 0, 5, 6, 7, 0},
        std::array{0, 0, 0, 0, 0, 8, 0},
        std::array{0, 0, 0, 0, 0, 0, 9},
        std::array{0, 0, 0, 0, 0, 0, 10},
        std::array{0, 0, 0, 0, 0, 0, 0},
    };

    Graph::Graph g{};
    size_t       count_of_edges = 0;
    for (size_t i = 0; i < nodes_count; ++i)
        for (size_t j = i + 1; j < nodes_count; ++j)
            if (const auto weight = adjacency_matrix[i][j])
            {
                g.AddEdge(std::to_string(i), std::to_string(j), weight);
                ++count_of_edges;
            }

    EXPECT_EQ(count_of_edges, g.GetEdgesCount());
    EXPECT_EQ(nodes_count, g.GetVertexesCount());

    g.ToFile("Test_1", s_show_graphs);

    std::set<Graph::EdgePtr> edges_to_delete{};
    g.ForEachVertex([&](const Graph::Vertex& vertex)
    {
        auto edges = vertex.GetEdges();
        if (!edges.empty())
            edges_to_delete.insert(*edges.begin());
    });

    for (const auto& edge : edges_to_delete)
        g.ContractEdge(edge);

    EXPECT_EQ(count_of_edges - edges_to_delete.size(), g.GetEdgesCount());
    EXPECT_EQ(1, g.GetVertexesCount());

    g.ToFile("Test_1_contracted", s_show_graphs);
}
