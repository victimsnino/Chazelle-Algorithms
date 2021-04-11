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

#include <algorithm>
#include <numeric>
#include <random>


static std::vector<std::vector<uint32_t>> GenerateMatrix(uint32_t k, uint32_t postprocess_)
{
    std::vector<std::pair<uint32_t, uint32_t>> edges{};
    uint32_t                                   N = 1;
    for (uint32_t i = 0; i < k; ++i)
    {
        std::vector<std::pair<uint32_t, uint32_t>> edges_to_add{};
        for (const auto [i,j] : edges)
        {
            edges_to_add.emplace_back(i + N, j + N);
        }
        std::move(edges_to_add.begin(), edges_to_add.end(), std::back_inserter(edges));
        edges.emplace_back(0, N);
        N *= 2;
    }
    for (uint32_t z = 0; z < postprocess_; ++z)
    {
        std::vector<std::pair<uint32_t, uint32_t>> edges_to_add{};
        std::for_each(edges.rbegin(),
                      edges.rend(),
                      [&](const auto& pair)
                      {
                          auto [i, j] = pair;
                          edges_to_add.emplace_back(i, N);
                          edges_to_add.emplace_back(j, N);
                          N += 1;
                      });
        std::move(edges_to_add.begin(), edges_to_add.end(), std::back_inserter(edges));
    }

    std::vector<std::vector<uint32_t>> result{N};
    for (size_t i = 0; i < N; ++i)
        result[i].resize(N);

    uint32_t weight = edges.size();
    for (const auto& [i,j] : edges)
    {
        result[i][j] = weight;
        result[j][i] = weight;
        --weight;
    }
    return result;
}

static std::vector<std::tuple<size_t, size_t, size_t>> ErdosRenie(uint32_t n, double p)
{
    std::vector<size_t> values(n, 0);
    std::iota(values.begin(), values.end(), 0);

    std::vector<size_t> weights(std::pow(n,2), 0);
    std::iota(weights.begin(), weights.end(), 1);

    std::random_device rd;
    std::mt19937       g(rd());
    std::ranges::shuffle(weights, g);

    std::vector<std::tuple<size_t, size_t, size_t>> result{};
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

std::vector<size_t> RunBoruvka(Graph::Graph&& g)
{
    std::vector<size_t> boruvka_result{};
    uint32_t count = 0;
    {
        Utils::MeasurePerfomance measure{ "Boruvka" };
        while (g.GetEdgesCount() != 0)
        {
            std::ranges::move(g.BoruvkaPhase(), std::back_inserter(boruvka_result));
            ++count;
        }
    }
    std::cout << "Required Boruvka stages: " << count << std::endl;
    return boruvka_result;
}

std::vector<size_t> RunMST(Graph::Graph& g)
{
    Utils::MeasurePerfomance measure{"SoftHeap MST"};
    return MST::FindMST(g);
}

void CompareBoruvkaAndMst(std::vector<size_t>& boruvka_result, std::vector<size_t>& mst_result)
{
    std::ranges::sort(mst_result);
    std::ranges::sort(boruvka_result);

    std::vector<size_t> diff{};
    std::ranges::set_symmetric_difference(mst_result, boruvka_result, std::back_inserter(diff));
    EXPECT_THAT(diff, ::testing::SizeIs(0));
}

//TEST(MST, TestGraph)
//{
//    //auto matrix = GenerateMatrix(12, 1);
//    auto matrix = GenerateMatrix(7, 5);
//    //auto matrix = GenerateMatrix(1, 3);
//    Graph::Graph g{matrix};
//    std::cout << "V: " << g.GetVerticesCount() << " E: " << g.GetEdgesCount() << std::endl;
//
//    auto boruvka_result = RunBoruvka(Graph::Graph{matrix});
//    auto mst_result     = RunMST(g);
//
//    CompareBoruvkaAndMst(boruvka_result, mst_result);
//}

TEST(MST, ErdosGraph)
{
    auto edges = ErdosRenie(500, 0.001);
    Graph::Graph g{edges};
    std::cout << "V: " << g.GetVerticesCount() << " E: " << g.GetEdgesCount() << std::endl;

    auto boruvka_result = RunBoruvka(Graph::Graph{edges});
    auto mst_result     = RunMST(g);

    CompareBoruvkaAndMst(boruvka_result, mst_result);

    auto lambda = [&](size_t result, size_t edge)
    {
        return std::get<2>(edges[edge]) + result;
    };
    auto boruvka = std::accumulate(boruvka_result.cbegin(), boruvka_result.cend(), 0, lambda);
    auto mst = std::accumulate(mst_result.cbegin(), mst_result.cend(), 0, lambda);

    std::cout << "Boruvka sum: " << boruvka << " Mst Sum: " << mst << " Boruvka == mst " << (boruvka == mst) << std::endl;
}
