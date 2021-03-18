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

#include "MSTTreeBase.h"

#include <Graph.h>
#include <SoftHeapCpp.h>

#include <set>


namespace MST
{
using Cluster = std::set<Details::EdgePtrWrapper>;

class MSTTree
{
public:
    MSTTree(Graph::Graph& graph, size_t c);

private:
    // Step 1,2: Boruvka
    bool BoruvkaPhase(size_t c) const;

    // Step 3: Building tree T
    void BuildTree();

private:
    bool Extension();
    void Retraction();

    void MoveItemsToSuitableHeapsByClusters(std::vector<Details::MSTSoftHeapDecorator>& cross_heaps,
                                            std::list<Details::EdgePtrWrapper>&&        valid_items);

    void InsertEdgeToCorrectHeap(Graph::Details::Edge&                       edge,
                                 bool                                        is_his_cluster_empty,
                                 std::vector<Details::MSTSoftHeapDecorator>& cross_heaps);


private:
    Details::MSTTreeBase m_base;
};
}
