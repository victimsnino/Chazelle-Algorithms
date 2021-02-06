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

#include <Graph.h>
#include <stdexcept>

static constexpr uint32_t S(uint32_t i, uint32_t j)
{
    if (i == 0 || j == 0)
        throw std::out_of_range{"i and j must be > 0"};

    if (i == 1)
        return 2 * j;
    if (j == 1)
        return 2;
    return S(i, j - 1) * S(i - 1, S(i, j - 1));
}

static_assert(S(1, 5) == 2 * 5);
static_assert(S(100500, 1) == 2);

static uint32_t FindT(const Graph::Graph& graph, double c)
{
    const double vertexes_count = graph.GetVertexesCount();
    uint32_t     d              = c * ceil(pow(static_cast<double>(graph.GetEdgesCount()) /
                                               vertexes_count,
                                               1.0 / 3.0));
    uint32_t result = 1;
    while (vertexes_count > pow(S(result, d), 3.0))
        ++result;

    return result;
}
