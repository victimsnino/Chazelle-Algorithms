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
    EdgePtrWrapper(const Graph::Details::Edge& edge, size_t outside_vertex)
        : m_edge{&edge}
        , m_outside_vertex{outside_vertex} {}

    const Graph::Details::Edge& GetEdge() const { return *m_edge; }
    const Graph::Details::Edge* operator->() const { return m_edge; }

    size_t GetOutsideVertex() const { return m_outside_vertex; }

    bool operator<(const EdgePtrWrapper& rhs) const { return m_working_cost < rhs.m_working_cost; }
    bool operator<=(const EdgePtrWrapper& rhs) const { return m_working_cost <= rhs.m_working_cost; }
    bool operator==(const EdgePtrWrapper& rhs) const { return m_edge == rhs.m_edge; }

    void   SetWorkingCost(size_t cost) { m_working_cost = cost; }
    size_t GetWorkingCost() const { return m_working_cost; }
private:
    const Graph::Details::Edge* const m_edge;
    size_t                            m_working_cost = m_edge->GetWeight();
    const size_t                      m_outside_vertex;
};

struct EdgePtrWrapperShared
{
    EdgePtrWrapperShared(const std::shared_ptr<EdgePtrWrapper>& value)
        : shared_pointer{value} {}

    bool operator<(const EdgePtrWrapperShared& rhs) const { return *shared_pointer < *rhs.shared_pointer; }
    bool operator==(const EdgePtrWrapperShared& rhs) const { return *shared_pointer == *rhs.shared_pointer; }

    std::shared_ptr<EdgePtrWrapper> shared_pointer;
};

class MSTSoftHeapDecorator
{
public:
    explicit MSTSoftHeapDecorator(size_t r);

    using ExtractedItems = ::ExtractedItems<EdgePtrWrapper>;

    void           Insert(EdgePtrWrapper new_key);
    void           Meld(MSTSoftHeapDecorator& other);
    ExtractedItems ExtractItems();

    EdgePtrWrapper  DeleteMin();
    EdgePtrWrapper* FindMin();

    std::list<EdgePtrWrapper> DeleteAndReturnIf(std::function<bool(const EdgePtrWrapper& edge)> func);

private:
    SoftHeapCpp<EdgePtrWrapperShared>          m_heap;
    std::list<std::shared_ptr<EdgePtrWrapper>> m_items{};
};
}
