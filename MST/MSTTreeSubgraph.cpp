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

#include "MSTTreeSubgraph.h"

#include <Common.h>


namespace MST::Details
{
SubGraph::SubGraph(size_t vertex, size_t level_in_tree, size_t target_size, size_t r)
    : m_vertex{vertex}
    , m_level_in_tree{level_in_tree}
    , m_target_size{target_size}
    , m_r{r}
{
    InitHeaps();
}

SubGraph::SubGraph(const SubGraphPtr& child, size_t target_size, size_t r)
    : m_level_in_tree{child->GetLevelInTree() - 1}
    , m_target_size{target_size}
    , m_r{r}
    , m_childs{{std::nullopt, child}}
{
    InitHeaps();
}

void SubGraph::InitHeaps()
{
    m_heaps.reserve(m_level_in_tree + 1);

    for (size_t i = 0; i < m_level_in_tree + 1; ++i)
        m_heaps.emplace_back(m_r);

    m_heaps.shrink_to_fit();
}

size_t SubGraph::GetLevelInTree() const
{
    return m_level_in_tree;
}

bool SubGraph::IsMeetTargetSize() const
{
    return m_vertex.has_value() ? true : m_childs.size() >= m_target_size;
}

std::list<size_t> SubGraph::GetVertices() const
{
    assert(m_vertex.has_value()  &&  m_childs.empty());
    assert(!m_vertex.has_value() && !m_childs.empty());

    if(m_vertex)
        return {*m_vertex};

    std::list<size_t> result{};
    for(const auto& child : m_childs)
        result.splice(result.end(), child.second->GetVertices());

    return result;
}

void SubGraph::PushToHeap(EdgePtrWrapper edge)
{
    m_heaps[m_level_in_tree].Insert(edge);
}

void SubGraph::MeldHeapsFrom(SubGraphPtr& other)
{
    // m_level_in_tree = other.m_level_in_tree - 1
    for (size_t i = 0; i < m_level_in_tree; ++i)
        m_heaps[i].Meld(other->m_heaps[i]);
}

void SubGraph::AddToMinLinks(const EdgePtrWrapper& edge)
{
    m_min_links_to_next_nodes_in_active_path.emplace_back(edge);
}

void SubGraph::PopMinLink()
{
    if (!m_min_links_to_next_nodes_in_active_path.empty())
        m_min_links_to_next_nodes_in_active_path.pop_back();
}

MSTSoftHeapDecorator::ExtractedItems SubGraph::ExtractItems()
{
    MSTSoftHeapDecorator::ExtractedItems data{};

    std::for_each_n(m_heaps.rbegin(),
                    std::min(size_t{2}, m_level_in_tree),
                    [&](MSTSoftHeapDecorator& heap)
                    {
                        auto heap_data = heap.ExtractItems();
                        data.corrupted.splice(data.corrupted.end(), heap_data.corrupted);
                        data.items.splice(data.items.end(), heap_data.items);
                    });

    m_heaps.clear();
    InitHeaps();

    return data;
}

void SubGraph::AddChild(size_t edge, const SubGraphPtr& child)
{
    m_childs.emplace_back(edge, child);
}

SubGraphPtr SubGraph::PopLastChild()
{
    assert(!m_childs.empty());

    auto value = std::move(m_childs.back().second);
    m_childs.pop_back();
    return value;
}

MSTSoftHeapDecorator* SubGraph::FindHeapWithMin()
{
    auto* min_heap  = &m_heaps.front();
    auto  min_value = min_heap->FindMin();
    for (auto& heap : m_heaps | rgv::drop(1))
    {
        const auto new_value = heap.FindMin();
        if (!min_value || new_value && *new_value < *min_value)
        {
            min_heap  = &heap;
            min_value = new_value;
        }
    }
    return min_value ? min_heap : nullptr;
}


std::list<EdgePtrWrapper> SubGraph::DeleteAndReturnIf(std::function<bool(const EdgePtrWrapper& edge)> func)
{
    std::list<EdgePtrWrapper> result{};
    for (auto& heap : m_heaps)
        result.splice(result.end(), heap.DeleteAndReturnIf(func));
    return result;
}
} // namespace MST::Details
