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

#include "MSTUtils.h"

#include <Graph.h>
//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include <spdlog/spdlog.h>

#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace MST
{
static uint32_t Ackermann(uint32_t i, uint32_t j)
{
    static std::map<uint32_t, std::map<uint32_t, std::optional<uint32_t>>> s_result{};
    auto& result = s_result[i][j];
    if (result.has_value())
        return result.value();

    if (i == 0)
        result = 2*j;
    else if (j == 0)
        result = 0;
    else if (j == 1)
        result = 2;
    else
        result = Ackermann(i - 1, Ackermann(i, j - 1));

    return result.value();
}


static uint32_t S(uint32_t i, uint32_t j)
{
    return Ackermann(i-1, j);
    static std::map<uint32_t, std::map<uint32_t, std::optional<uint32_t>>> s_result{};

    auto& result = s_result[i][j];
    if (i == 0 || j == 0)
        throw std::out_of_range{"i and j must be > 0"};

    if (i == 1)
        result = Ackermann(i-1, j);
    else if (j == 1)
        result = Ackermann(i-1, j);
    else
        result = S(i, j - 1)*S(i - 1, S(i, j - 1));

    return result.value();
}

//static_assert(S(1, 1) == 2);
//static_assert(S(3, 1) == 2);

uint32_t FindMaxHeight(Graph::Graph& graph, uint32_t c)
{
    const double vertexes_count = static_cast<double>(graph.GetVerticesCount());
    const double edges_count    = static_cast<double>(graph.GetTotalEdgesCount());
    return c * static_cast<uint32_t>(std::ceil(std::pow(edges_count / vertexes_count, 1.0 / 3.0)));
}

uint32_t FindParamT(const Graph::Graph& graph, uint32_t d)
{
    if (d == 1)
        return 1;

    const double vertexes_count = static_cast<double>(graph.GetVerticesCount());

    uint32_t result = 1;
    SPDLOG_DEBUG("std::pow(S(result, d), 3) {}", std::pow(S(result, d), 3));
    while (vertexes_count > std::pow(S(result, d), 3))
        ++result;

    return result;
}

uint32_t CalculateTargetSize(uint32_t t, uint32_t node_height)
{
    if (node_height == 1)
        return std::pow(S(t, 1u), 3u);
    return std::pow(S(t - 1, S(t, node_height - 1)), 3u);
}
} // namespace MST
