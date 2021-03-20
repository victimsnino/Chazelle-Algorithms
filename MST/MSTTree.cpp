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

#include "MSTTree.h"

#include "MSTUtils.h"

#include <Graph.h>


namespace MST
{
MSTTree::MSTTree(Graph::Graph& graph, size_t c)
    : m_graph{graph}
    , m_stack{m_graph, c}
{
    while(true)
    {
        if (m_stack.top().IsMeetTargetSize())
        {
            if (!Retraction())
                return;
        }
        else
        {
            if (!Extension())
                return;
        }
    }
}

bool MSTTree::Retraction()
{
    auto& last_node = m_stack.top();
    if (last_node.GetIndex() == 0)
        return false;

    auto data = m_stack.Pop();
    for (auto& corrupted : data.corrupted)
        m_graph.DisableEdge(corrupted->GetIndex());

    return true;
}

bool MSTTree::Extension()
{

    return false;
}

MSTTree MSTTree::Create(Graph::Graph& graph, size_t c)
{
    // TODO: ENable boruvka Phase later
    //size_t count = FindParamT(graph, FindMaxHeight(graph, c)) == 1 ? std::numeric_limits<uint32_t>::max() : c;

    //while (count > 0 && graph.GetVertexesCount() > 1)
    //{
    //    graph.BoruvkaPhase();
    //    --count;
    //}

    //if (graph.GetVertexesCount() == 1)
    //    return false;
    //return true;
    return MSTTree{graph, c};
}
} // namespace MST
