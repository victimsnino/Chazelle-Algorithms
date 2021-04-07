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

TEST(MST, Init)
{
    auto matrix = GenerateMatrix(7, 5);
    //auto matrix = GenerateMatrix(1, 3);
    std::vector<size_t> boruvka_result{};
    {
        Graph::Graph g{matrix};
        std::cout << g.GetVerticesCount() << " " << g.GetEdgesCount() << std::endl;
        uint32_t count = 0;
        {
            Utils::MeasurePerfomance measure{"Boruvka"};
            while (g.GetVerticesCount() != 1)
            {
                std::ranges::move(g.BoruvkaPhase(), std::back_inserter(boruvka_result));
                ++count;
            }
        }
        std::cout << "Required Boruvka stages: " << count << std::endl;
    }

    Graph::Graph g{matrix};
    //ToFile(g, "TEMP-Pre", true);

    std::vector<size_t> mst_result;
    {
        Utils::MeasurePerfomance measure{"SoftHeap MST"};
        mst_result = MST::FindMST(g);
    }

    std::ranges::sort(mst_result);
    std::ranges::sort(boruvka_result);

    std::vector<size_t> diff{};
    std::ranges::set_symmetric_difference(mst_result, boruvka_result, std::back_inserter(diff));
    EXPECT_THAT(diff, ::testing::SizeIs(0));
}
