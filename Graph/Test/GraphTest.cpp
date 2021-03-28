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

#include <Common.h>
#include <Graph.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <array>
#include <ranges>

static constexpr  bool       s_show_graphs = false;

static std::vector<std::vector<uint32_t>> s_adjacency_matrix{
    {0,  4, 11, 3, 0, 0, 0},
    {4,  0, 1, 12, 2, 0, 0},
    {11, 1, 0, 5, 6, 7, 0},
    {3, 12, 5, 0, 0, 8, 0},
    {0,  2, 6, 0, 0, 0, 9},
    {0,  0, 7, 8, 0, 0, 10},
    {0,  0, 0, 0, 9, 10, 0},
};


std::tuple<Graph::Graph, uint32_t, size_t> FillGraph(const std::vector<std::vector<uint32_t>>& matrix)
{
    Graph::Graph g{ matrix };
    size_t       count_of_edges = 0;
    for (size_t i = 0; i < matrix.size(); ++i)
        for (size_t j = 0; j < i; ++j)
            if (const auto weight = matrix[i][j])
                ++count_of_edges;

    EXPECT_EQ(count_of_edges, g.GetEdgesCount());
    EXPECT_EQ(matrix.size(), g.GetVerticesCount());
    return std::make_tuple(g, count_of_edges, matrix.size());
}

TEST(Graph, DummyChecks)
{
    {
        Graph::Graph g{};
        EXPECT_EQ(g.GetEdgesCount(), 0);
        EXPECT_EQ(g.GetVerticesCount(), 0);

        g.BoruvkaPhase();

        EXPECT_EQ(g.GetEdgesCount(), 0);
        EXPECT_EQ(g.GetVerticesCount(), 0);
    }

    Graph::Graph g{std::vector<std::vector<uint32_t>>{{0, 1},
                                                      {1, 0}}};

    EXPECT_EQ(g.GetEdgesCount(), 1);
    EXPECT_EQ(g.GetVerticesCount(), 2);

    for (int i = 0; i < 2; ++i)
    {
        EXPECT_THAT(g.BoruvkaPhase(), ::testing::SizeIs(1-i));
        EXPECT_EQ(g.GetEdgesCount(), 0);
        EXPECT_EQ(g.GetVerticesCount(), 1);
    }
}

TEST(Graph, Init)
{
    auto [g, count_of_edges, nodes_count] = FillGraph(s_adjacency_matrix);

    ToFile(g, "Test_1", s_show_graphs);

    g.ContractEdge(0);

    EXPECT_EQ(count_of_edges-3, g.GetEdgesCount());
    EXPECT_EQ(nodes_count -1, g.GetVerticesCount());

    ToFile(g, "Test_1_contracted", s_show_graphs);
}

TEST(Graph, Boruvka)
{
    auto [g, count_of_edges, nodes_count] = FillGraph(s_adjacency_matrix);

    ToFile(g, "Test_2", s_show_graphs);

    auto mst_indexes = g.BoruvkaPhase();
    std::vector<std::array<size_t, 2>> mst_edges{};
    for(auto index : mst_indexes)
        mst_edges.emplace_back(g.GetEdge(index).GetOriginalVertices());

    const std::set<std::array<size_t, 2>> contracted{{0, 3},
                                                     {1, 2},
                                                     {1, 4},
                                                     {2, 5},
                                                     {4, 6}};
    const size_t removed_edges = contracted.size();

    EXPECT_THAT(mst_edges, ::testing::SizeIs(contracted.size()));
    for (const auto& vertexes : mst_edges)
        EXPECT_TRUE(contracted.count(vertexes));

    EXPECT_EQ(1, g.GetEdgesCount());
    EXPECT_EQ(nodes_count- removed_edges, g.GetVerticesCount());

    ToFile(g, "Test_2_contracted", s_show_graphs, false);
    ToFile(g, "Test_2_contracted_mst", s_show_graphs, true);

    g.BoruvkaPhase();

    EXPECT_EQ(0, g.GetEdgesCount());
    EXPECT_EQ(1, g.GetVerticesCount());
    ToFile(g, "Test_2_contracted_mst_final", s_show_graphs, true);
}

static std::vector<std::vector<uint32_t>> s_extended_adjacency_matrix{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 2, 0, 0, 0, 0, 0, 0, 0, 0},
    {3, 4, 5, 0, 0, 0, 0, 0, 0, 0},
    {0, 6, 7, 0, 0, 0, 0, 0, 0, 0},
    {8, 0, 9, 0, 10, 0, 0, 0, 0, 0},
    {11, 12, 0, 13, 0, 0, 0, 0, 0, 0},
    {0, 14, 15, 0, 16, 0, 17, 0, 0, 0},
    {18, 0, 0, 19, 0, 20, 0, 21, 0, 0},
    {0, 22, 0, 24, 0, 25, 0, 30, 35, 0},
};

TEST(Graph, Boruvka_2)
{
    auto [g, count_of_edges, nodes_count] = FillGraph(s_extended_adjacency_matrix);

    ToFile(g, "Test_3", s_show_graphs);

    auto mst = g.BoruvkaPhase();

    std::set<std::array<size_t, 2>> contracted{ {0, 3},
                                                        {0, 1},
                                                        {1, 2},
                                                        {1, 4},
                                                        {0, 5},
                                                        {0, 6},
                                                        {1, 7},
                                                        {0, 8},
                                                        {1, 9},

    };
    size_t removed_edges = contracted.size();

    EXPECT_EQ(mst.size(), contracted.size());
    for (const auto& vertexes : mst)
        EXPECT_TRUE(contracted.count(g.GetEdge(vertexes).GetOriginalVertices()));

    EXPECT_EQ(0, g.GetEdgesCount());
    EXPECT_EQ(1, g.GetVerticesCount());

    ToFile(g, "Test_3_contracted", s_show_graphs, false);
    ToFile(g, "Test_3_contracted_mst", s_show_graphs, true);

    g.BoruvkaPhase();

    EXPECT_EQ(0, g.GetEdgesCount());
    EXPECT_EQ(1, g.GetVerticesCount());
    ToFile(g, "Test_3_contracted_mst_final", s_show_graphs, true);
}