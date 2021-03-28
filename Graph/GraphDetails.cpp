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

#include "GraphDetails.h"

#include "Graph.h"

#include <cassert>

namespace Graph::Details
{
Edge::Edge(const MemberOfSubGraphPtr& i,
           const MemberOfSubGraphPtr& j,
           uint32_t                   weight,
           size_t                     index)
    : m_i(i)
    , m_j(j)
    , m_weight(weight)
    , m_index(index) { }

Edge::Edge(Edge&& other) noexcept
    : m_i{other.m_i}
    , m_j{other.m_j}
    , m_weight{other.m_weight}
    , m_index{other.m_index}
    , m_is_contracted{other.m_is_contracted}
    , m_is_disabled{other.m_is_disabled} {}

Edge& Edge::operator=(Edge&& other) noexcept
{
    if (this == &other)
        return *this;
    m_i             = other.m_i;
    m_j             = other.m_j;
    m_weight        = other.m_weight;
    m_index         = other.m_index;
    m_is_contracted = other.m_is_contracted;
    m_is_disabled   = other.m_is_disabled;
    return *this;
}

std::array<size_t, 2> Edge::GetOriginalVertices() const
{
    return {m_i->GetOrignal(), m_j->GetOrignal()};
}

std::array<size_t, 2> Edge::GetCurrentSubgraphs() const
{
    return {m_i->GetParent(), m_j->GetParent()};
}

MemberOfSubGraph::MemberOfSubGraph(size_t parent, size_t rank)
    : m_parent(parent)
    , m_original_vertex(parent)
    , m_rank(rank) {}

void EdgesView::AddEdge(const MemberOfSubGraphPtr& begin,
                        const MemberOfSubGraphPtr& end,
                        uint32_t                   weight)
{
    m_indexes.push_back(m_edges.size());
    m_edges.emplace_back(begin, end, weight, m_indexes.back());
}

void EdgesView::ContractEdge(size_t index)
{
    m_edges[index].SetIsContracted();
    DisableEdge(index);
}

void EdgesView::DisableEdge(size_t index)
{
    m_indexes.remove(index);
    m_edges[index].SetIsDisabled();
}

Edge& EdgesView::operator[](size_t index)
{
    assert(index < m_edges.size());
    return m_edges[index];
}

const Edge& EdgesView::operator[](size_t index) const
{
    assert(index < m_edges.size());
    return m_edges[index];
}
} // namespace Graph::Details
