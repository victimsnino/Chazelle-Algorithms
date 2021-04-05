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

#pragma once
#include "MSTTree.h"

#include <Graph.h>

namespace MST
{
class MSTTreeBuilder
{
public:
    MSTTreeBuilder(Graph::Details::EdgesView& edges, size_t t, size_t max_height);

    MST::Details::MSTTree& GetTree() { return m_tree; }
    std::vector<size_t>&   GetBadEdges() { return m_bad_edges; }
private:
    // Pop last node from stack, discard corrupted edges, for rest create clusters and insert cheapest to heap
    bool Retraction();
    bool Extension();

    void                           CreateClustersAndPushCheapest(std::list<Details::EdgePtrWrapper>&& items);
    Details::MSTSoftHeapDecorator* FindHeapWithExtensionEdge();

    Details::MSTSoftHeapDecorator::ExtractedItems Fusion(Details::EdgePtrWrapper& edge);
    void PostRetractionActions(Details::MSTSoftHeapDecorator::ExtractedItems items);
private:
    Graph::Details::EdgesView& m_edges;
    Details::MSTTree           m_tree;
    std::vector<size_t>        m_bad_edges{};
};
}
