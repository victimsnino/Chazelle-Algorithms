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

static constexpr  bool       s_show_graphs = false;

constexpr std::array s_adjacency_matrix{
    std::array{0,  4, 11, 3, 0, 0, 0},
    std::array{4,  0, 1, 12, 2, 0, 0},
    std::array{11, 1, 0, 5, 6, 7, 0},
    std::array{3, 12, 5, 0, 0, 8, 0},
    std::array{0,  2, 6, 0, 0, 0, 9},
    std::array{0,  0, 7, 8, 0, 0, 10},
    std::array{0,  0, 0, 0, 9, 10, 0},
};

static_assert(s_adjacency_matrix.size() == s_adjacency_matrix[0].size());

template<size_t nodes_count>
std::tuple<Graph::Graph, uint32_t, uint32_t> FillGraph(const std::array<std::array<int, nodes_count>, nodes_count>& matrix)
{
    Graph::Graph g{ matrix };
    size_t       count_of_edges = 0;
    for (size_t i = 0; i < nodes_count; ++i)
        for (size_t j = 0; j < i; ++j)
            if (const auto weight = matrix[i][j])
                ++count_of_edges;

    EXPECT_EQ(count_of_edges, g.GetEdgesCount());
    EXPECT_EQ(nodes_count, g.GetVertexesCount());
    return { g, count_of_edges, nodes_count };
}

TEST(Graph, DummyChecks)
{
    {
        Graph::Graph g{};
        EXPECT_EQ(g.GetEdgesCount(), 0);
        EXPECT_EQ(g.GetVertexesCount(), 0);
        EXPECT_EQ(g.GetMST().size(), 0);

        g.BoruvkaPhase();

        EXPECT_EQ(g.GetEdgesCount(), 0);
        EXPECT_EQ(g.GetVertexesCount(), 0);
        EXPECT_EQ(g.GetMST().size(), 0);
    }

    Graph::Graph g{ std::array{std::array{0, 1},
                               std::array{1, 0}}};

    EXPECT_EQ(g.GetEdgesCount(), 1);
    EXPECT_EQ(g.GetVertexesCount(), 2);
    EXPECT_EQ(g.GetMST().size(), 0);

    for (int i = 0; i < 2; ++i)
    {
        g.BoruvkaPhase();

        EXPECT_EQ(g.GetEdgesCount(), 0);
        EXPECT_EQ(g.GetVertexesCount(), 1);
        EXPECT_EQ(g.GetMST().size(), 1);
    }
}

TEST(Graph, Init)
{
    auto [g, count_of_edges, nodes_count] = FillGraph(s_adjacency_matrix);

    ToFile(g, "Test_1", s_show_graphs);

    g.ContractEdge(0);

    EXPECT_EQ(count_of_edges-3, g.GetEdgesCount());
    EXPECT_EQ(nodes_count -1, g.GetVertexesCount());

    const std::set<uint32_t>   rest_vertexes{0, 2, 3, 4, 5, 6}; // 1 contracted to 0
    std::set<uint32_t>         actual_vertices{};
   /* g.ForEachEdge([&](uint32_t i, uint32_t j, uint32_t weight)
    {
        actual_vertices.insert(i);
        actual_vertices.insert(j);
    });

    EXPECT_EQ(rest_vertexes, actual_vertices);*/

    ToFile(g, "Test_1_contracted", s_show_graphs);
}

//TEST(Graph, Boruvka)
//{
//    auto [g, count_of_edges, nodes_count] = FillGraph(s_adjacency_matrix);
//
//    ToFile(g, "Test_2", s_show_graphs);
//
//    g.BoruvkaPhase();
//
//    std::set<std::pair<uint32_t, uint32_t>> contracted{Graph::BuildPairForEdge(0, 3),
//                                                       Graph::BuildPairForEdge(1, 2),
//                                                       Graph::BuildPairForEdge(2, 1),
//                                                       Graph::BuildPairForEdge(3, 0),
//                                                       Graph::BuildPairForEdge(4, 1),
//                                                       Graph::BuildPairForEdge(5, 2),
//                                                       Graph::BuildPairForEdge(6, 4)};
//    size_t removed_edges = contracted.size();
//
//    auto mst = g.GetMST();
//    EXPECT_EQ(mst.size(), contracted.size());
//    for (const auto& edge_mst : mst)
//        EXPECT_TRUE(contracted.count(edge_mst));
//
//    EXPECT_EQ(1, g.GetEdgesCount());
//    EXPECT_EQ(nodes_count- removed_edges, g.GetVertexesCount());
//
//    ToFile(g, "Test_2_contracted", s_show_graphs, false);
//    ToFile(g, "Test_2_contracted_mst", s_show_graphs, true);
//
//    BoruvkaPhase(g);
//
//    EXPECT_EQ(0, g.GetEdgesCount());
//    EXPECT_EQ(1, g.GetVertexesCount());
//    ToFile(g, "Test_2_contracted_mst_final", s_show_graphs, true);
//}
//
//constexpr std::array s_extended_adjacency_matrix{
//    std::array{-1,  0,  0,  0,  0,  0,  0,  0,  0,   0},
//    std::array{ 1, -1,  0,  0,  0,  0,  0,  0,  0,   0},
//    std::array{ 0,  2, -1,  0,  0,  0,  0,  0,  0,   0},
//    std::array{ 3,  4,  5, -1,  0,  0,  0,  0,  0,   0},
//    std::array{ 0,  6,  7,  0, -1,  0,  0,  0,  0,   0},
//    std::array{ 8,  0,  9,  0, 10, -1,  0,  0,  0,   0},
//    std::array{11, 12,  0, 13,  0,  0, -1,  0,  0,   0},
//    std::array{ 0, 14, 15,  0, 16,  0, 17, -1,  0,   0},
//    std::array{18,  0,  0, 19,  0, 20,  0, 21, -1,   0},
//    std::array{ 0, 22,  0, 24,  0, 25,  0, 30, 35, - 1},
//};
//
//static_assert(s_extended_adjacency_matrix.size() == s_extended_adjacency_matrix[0].size());
//
//TEST(Graph, Boruvka_2)
//{
//    auto [g, count_of_edges, nodes_count] = FillGraph(s_extended_adjacency_matrix);
//
//    ToFile(g, "Test_3", s_show_graphs);
//
//    BoruvkaPhase(g);
//
//    std::set<std::pair<uint32_t, uint32_t>> contracted{ Graph::BuildPairForEdge(1, 0),
//                                                        Graph::BuildPairForEdge(2, 1),
//                                                        Graph::BuildPairForEdge(3, 0),
//                                                        Graph::BuildPairForEdge(4, 1),
//                                                        Graph::BuildPairForEdge(5, 0),
//                                                        Graph::BuildPairForEdge(6, 0),
//                                                        Graph::BuildPairForEdge(7, 1),
//                                                        Graph::BuildPairForEdge(8, 0),
//                                                        Graph::BuildPairForEdge(9, 1),
//
//    };
//    size_t removed_edges = contracted.size();
//
//    auto mst = g.GetMST();
//    EXPECT_EQ(mst.size(), contracted.size());
//    for (const auto& edge_mst : mst)
//        EXPECT_TRUE(contracted.count(edge_mst));
//
//    EXPECT_EQ(0, g.GetEdgesCount());
//    EXPECT_EQ(1, g.GetVertexesCount());
//
//    ToFile(g, "Test_3_contracted", s_show_graphs, false);
//    ToFile(g, "Test_3_contracted_mst", s_show_graphs, true);
//
//    BoruvkaPhase(g);
//
//    EXPECT_EQ(0, g.GetEdgesCount());
//    EXPECT_EQ(1, g.GetVertexesCount());
//    ToFile(g, "Test_3_contracted_mst_final", s_show_graphs, true);
//}