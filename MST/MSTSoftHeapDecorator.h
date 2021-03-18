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
#include <GraphDetails.h>
#include <SoftHeapCpp.h>

#include <array>
#include <optional>


namespace MST::Details
{
using Label = std::array<std::optional<size_t>, 2>;

class EdgePtrWrapper
{
public:
    EdgePtrWrapper(const Graph::Details::Edge& edge)
        : m_edge{&edge} {}

    const Graph::Details::Edge& GetEdge() const { return *m_edge; }
    const Graph::Details::Edge* operator->() const { return m_edge; }

    void  SetLastHeapIndex(Label label) { m_last_heap_index = label; }
    Label GetLastHeapIndex() const { return m_last_heap_index; }

    bool operator<(const EdgePtrWrapper& rhs) const { return *m_edge < *rhs.m_edge; }
    bool operator==(const EdgePtrWrapper& rhs) const { return m_edge == rhs.m_edge; }
private:
    const Graph::Details::Edge* const m_edge;
    Label                             m_last_heap_index{};
};

class MSTSoftHeapDecorator : private SoftHeapCpp<EdgePtrWrapper>
{
public:
    MSTSoftHeapDecorator(size_t r, size_t label_i, std::optional<size_t> label_j = {});

    void                             Insert(EdgePtrWrapper new_key) override;
    EdgePtrWrapper                   DeleteMin() override;
    void                             Meld(MSTSoftHeapDecorator& other);
    const std::list<EdgePtrWrapper>& GetItemsInside() const { return m_items; }

    using SoftHeapCpp<EdgePtrWrapper>::ExtractedItems;
    using SoftHeapCpp<EdgePtrWrapper>::ExtractItems;
    using SoftHeapCpp<EdgePtrWrapper>::FindMin;

private:
    const Label               m_label;
    std::list<EdgePtrWrapper> m_items{};
};
}
