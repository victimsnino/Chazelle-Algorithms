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
#include "MSTSoftHeapDecorator.h"

#include <optional>
#include <ranges>

namespace rg = std::ranges;
namespace rgv = std::ranges::views;

namespace MST::Details
{
class SubGraph
{
public:
    SubGraph(std::list<size_t>::const_iterator vertex_itr, size_t index, size_t target_size, size_t r);

    SubGraph(SubGraph&& other)                 = delete;
    SubGraph(const SubGraph& other)            = delete;
    SubGraph& operator=(const SubGraph& other) = delete;
    SubGraph& operator=(SubGraph&& other)      = delete;

    void                      PushToHeap(EdgePtrWrapper edge);
    std::list<EdgePtrWrapper> DeleteAndReturnIf(std::function<bool(const EdgePtrWrapper& edge)> func);
    void Meld(SubGraph& other);
    SoftHeapCpp<EdgePtrWrapper>::ExtractedItems ExtractItems();

    void                      Contract(Graph::Graph& graph, std::list<size_t>& vertices_inside_tree);

    void AddToMinLinks(EdgePtrWrapper edge);
    void PopMinLink(bool chain_link = false);

    bool   IsMeetTargetSize() const { return GetSizeOfVertices() >= m_target_size; }
    size_t GetIndex() const { return m_index; }
    auto   GetVertices() const { return rgv::counted(m_vertices_begin, GetSizeOfVertices()); }
private:
    size_t GetSizeOfVertices() const { return std::distance(m_vertices_begin, m_vertices_end); }

private:
    const size_t m_index; // aka k
    const size_t m_target_size;

    const std::list<size_t>::const_iterator m_vertices_begin;
    const std::list<size_t>::const_iterator m_vertices_end;
    // technically it will be cend every time. as aa result, first node includes whole range, next lower subrange and etc

    std::vector<MSTSoftHeapDecorator> m_heaps; // i < m_index -> H(i, m_index) cross heap, else - H(m_index)

    std::optional<EdgePtrWrapper> m_chain_link_to_next{};
    std::vector<EdgePtrWrapper>   m_min_links_to_next_nodes{};
};
}
