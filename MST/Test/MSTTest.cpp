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
#include "MST.h"

#include <Common.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <Kruskal.h>

#include <algorithm>
#include <numeric>
#include <random>


static std::vector<std::tuple<size_t, size_t, size_t>> GenerateMatrix(uint32_t k, uint32_t postprocess_)
{
    std::vector<std::tuple<size_t, size_t, size_t>> edges{};
    uint32_t                                   N = 1;
    for (uint32_t i = 0; i < k; ++i)
    {
        auto size = edges.size();
        for (size_t count = 0; count < size; ++count)
        {
            auto [i,j,w] = edges[count];
            edges.emplace_back(i + N, j + N, 0);
        }
        edges.emplace_back(0, N, 0);
        N *= 2;
    }
    for (uint32_t z = 0; z < postprocess_; ++z)
    {
        for (int count = edges.size() - 1; count >= 0; count--)
        {
            auto [i,j,w] = edges[count];
            edges.emplace_back(i, N, 0);
            edges.emplace_back(j, N, 0);
            N += 1;
        }
    }

    std::vector<std::vector<uint32_t>> result{N};
    for (size_t i = 0; i < N; ++i)
        result[i].resize(N);

    uint32_t weight = edges.size();
    for (auto& edge : edges)
    {
       std::get<2>(edge) = weight--;
    }
    return edges;
}

static std::vector<std::tuple<size_t, size_t, size_t>> ErdosRenie(uint32_t n, double p)
{
    std::vector<size_t> values(n, 0);
    std::iota(values.begin(), values.end(), 0);

    std::vector<size_t> weights(std::pow(n,2), 0);
    std::iota(weights.begin(), weights.end(), 1);

    std::random_device rd;
    std::mt19937       g(1);
    std::ranges::shuffle(weights, g);

    std::vector<std::tuple<size_t, size_t, size_t>> result{};
    result.reserve(weights.size());
    std::uniform_real_distribution<>                dis(0.0, 1.0);
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = i + 1; j < n; ++j)
        {
            if (dis(g) <= p)
            {
                result.emplace_back(i, j, weights.back());
                weights.pop_back();
            }
        }
    }

    return result;
}

std::list<size_t> RunBoruvka(Graph::Graph&& g)
{
    Utils::MeasurePerfomance measure{ "Boruvka" };
    return g.BoruvkaPhase(100500);
}

auto RunMST(Graph::Graph& g)
{
    Utils::MeasurePerfomance measure{"SoftHeap MST"};
    return MST::FindMST(g);
}

auto RunKruskal(const std::vector<std::tuple<size_t, size_t, size_t>>& edges, size_t v)
{
    Kruskal::Graph g{v, edges.size()};
    size_t count = 0;
    for(auto& [i,j,w]: edges)
        g.addEdge(i,j,w, count++);

    Utils::MeasurePerfomance measure{"Kruskal"};
    return g.kruskalMST();
}


void CompareBoruvkaAndMst(std::list<size_t>& boruvka_result, std::list<size_t>& mst_result)
{
    mst_result.sort();
    boruvka_result.sort();

    std::vector<size_t> diff_in_boruvka, diff_in_mst{};
    std::ranges::set_difference(boruvka_result, mst_result, std::back_inserter(diff_in_boruvka));
    std::ranges::set_difference(mst_result, boruvka_result, std::back_inserter(diff_in_mst));
    EXPECT_EQ(mst_result.size(), boruvka_result.size());
    EXPECT_THAT(diff_in_boruvka, ::testing::SizeIs(0));
    EXPECT_THAT(diff_in_mst, ::testing::SizeIs(0));
}

//TEST(MST, TestGraph)
//{
//    //auto edges = GenerateMatrix(12, 1);
//    auto edges = GenerateMatrix(7, 5);
//    //auto edges = GenerateMatrix(6, 5);
//    //auto edges = GenerateMatrix(4, 6);
//    Graph::Graph g{edges};
//    size_t       v = g.GetVerticesCount();
//    std::cout << "V: " << v << " E: " << g.GetEdgesCount() << std::endl;
//
//    auto boruvka_result = RunBoruvka(Graph::Graph{edges});
//    auto mst_result     = RunMST(g);
//    auto kruskal_result = RunKruskal(edges, v);
//
//    CompareBoruvkaAndMst(boruvka_result, mst_result);
//    CompareBoruvkaAndMst(boruvka_result, kruskal_result);
//}

//TEST(MST, ErdosGraph)
//{
//    //auto edges = ErdosRenie(3000, 0.001); // c == 1 -> fail
//    //auto edges = ErdosRenie(23, 0.16); // c == 1 + Height
//    //auto edges = ErdosRenie(190, 0.02); // c == 1 + Height
//    //auto edges = ErdosRenie(500, 0.001);
//    auto         edges = ErdosRenie(10000, 0.0001);
//    Graph::Graph g{edges};
//    size_t       v           = g.GetVerticesCount();
//    size_t       e = g.GetEdgesCount();
//    //Graph::ToFile(g, "www", true, false);
//    std::cout << "V: " << v << " E: " << e << std::endl;
//
//    auto boruvka_result = RunBoruvka(Graph::Graph{edges});
//    auto mst_result     = RunMST(g);
//    //auto kruskal_result = RunKruskal(edges, v);
//
//    CompareBoruvkaAndMst(boruvka_result, mst_result);
//    //CompareBoruvkaAndMst(boruvka_result, kruskal_result);
//
//    /*auto lambda = [&](size_t result, size_t edge)
//    {
//        return std::get<2>(edges[edge]) + result;
//    };
//    auto boruvka = std::accumulate(boruvka_result.cbegin(), boruvka_result.cend(), 0, lambda);
//    auto mst = std::accumulate(mst_result.cbegin(), mst_result.cend(), 0, lambda);
//
//    std::cout << "Boruvka sum: " << boruvka << " Mst Sum: " << mst << " Boruvka == mst " << (boruvka == mst) << std::endl;*/
//}
