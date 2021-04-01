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

#include <Graph.h>

#include <iterator>


namespace MST
{
std::vector<size_t> MSF(Graph::Graph& graph, size_t max_height)
{
    size_t t = FindParamT(graph, max_height);
    size_t count = t == 1 ? std::numeric_limits<uint32_t>::max() : c;

    std::vector<size_t> boruvka_result{};
    while (count > 0 && graph.GetVerticesCount() > 1)
    {
        std::ranges::move(graph.BoruvkaPhase(), std::back_inserter(boruvka_result));
        --count;
    }

    //if (graph.GetVertexesCount() == 1)
        return boruvka_result;

    auto tree = MSTTreeBuilder(graph, t, max_height);

    //std::vector<size_t> F{};
    //for(auto& subgraph : tree)
    //    std::ranges::move(MSF(subgraph - tree.corrupted(), t-1), std::back_inserter(F));

    //return MSF(F + B, t) + boruvka_result;
}

std::vector<size_t> FindMST(Graph::Graph& graph)
{
    return MSF(graph, FindMaxHeight(graph, c));
}
} // namespace MST
