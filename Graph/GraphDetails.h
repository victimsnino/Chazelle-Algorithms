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

#include <array>
#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>

namespace Graph
{
class Graph;
}

namespace Graph::Details
{
class MemberOfSubGraph;
using MemberOfSubGraphPtr = std::shared_ptr<MemberOfSubGraph>;

class Edge
{
public:
    Edge(const MemberOfSubGraphPtr& i,
         const MemberOfSubGraphPtr& j,
         uint32_t                   weight,
         size_t                     index,
         std::optional<size_t> original_index = {});

    Edge(const Edge& other)            = default;
    Edge& operator=(const Edge& other) = default;

    Edge(Edge&& other) noexcept            = default;
    Edge& operator=(Edge&& other) noexcept = default;

    std::array<size_t, 2> GetOriginalVertices() const;
    std::array<size_t, 2> GetCurrentSubgraphs() const;

    size_t GetIndex() const { return m_index; }

    size_t GetOriginalIndex() const
    {
        return m_original_index.has_value() ? m_original_index.value() : m_index;
    }

    uint32_t GetWeight() const { return m_weight; }

    void SetIsContracted() { m_is_contracted = true; }
    bool IsContracted() const { return m_is_contracted; }

    void SetIsDisabled() { m_is_disabled = true; }
    bool IsDisabled() const { return m_is_disabled; }

    bool operator<(const Edge& rhs) const { return m_weight < rhs.m_weight; }
    bool operator==(const Edge& rhs) const { return m_index == rhs.m_index; }
private:
    MemberOfSubGraphPtr   m_i;
    MemberOfSubGraphPtr   m_j;
    uint32_t              m_weight;
    size_t                m_index;
    bool                  m_is_contracted{false};
    bool                  m_is_disabled{false};
    std::optional<size_t> m_original_index;
};

class MemberOfSubGraph
{
public:
    MemberOfSubGraph(size_t parent, size_t rank = 0);

    size_t GetOrignal() const { return m_original_vertex; }
    size_t GetParent() const { return m_parent; }
    void   SetParent(size_t parent) { m_parent = parent; }

    bool IsRoot() const { return GetParent() == GetOrignal(); }

    // Equal or not by parents (AKA same tree or not)
    bool operator==(const MemberOfSubGraph& rhs) const { return m_parent == rhs.m_parent; }
    bool operator!=(const MemberOfSubGraph& rhs) const { return !(*this == rhs); }

    bool operator<(const MemberOfSubGraph& rhs) const { return m_rank < rhs.m_rank; }
private:
    size_t       m_parent{};
    const size_t m_original_vertex;
    size_t       m_rank{}; // unused in case of m_parent != i
};

struct EdgesView
{
    EdgesView() = default;

    template<bool is_const>
    struct iterator
    {
        using Container = std::conditional_t<
            is_const, const std::vector<Edge>&, std::vector<Edge>&>;

        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::conditional_t<is_const, const Edge, Edge>;
        using pointer = value_type*;
        using reference = value_type&;

        iterator(Container edges, std::list<size_t>::const_iterator itr)
            : m_edges(edges)
            , m_itr(std::move(itr)) {}

        iterator(const iterator& other)
            : m_edges{other.m_edges}
            , m_itr{other.m_itr} {}

        iterator& operator=(const iterator& other)
        {
            if (this == &other)
                return *this;
            m_edges = other.m_edges;
            m_itr   = other.m_itr;
            return *this;
        }

        void      operator ++() { ++m_itr; }
        bool      operator ==(const iterator& other) const { return m_itr == other.m_itr; }
        bool      operator !=(const iterator& other) const { return m_itr != other.m_itr; }
        reference operator*() const { return m_edges[*m_itr]; }
    private:
        Container                         m_edges;
        std::list<size_t>::const_iterator m_itr;
    };

    void AddEdge(const MemberOfSubGraphPtr& begin,
                 const MemberOfSubGraphPtr& end,
                 uint32_t                   weight,
                 std::optional<size_t> original_index = {});

    void ContractEdge(size_t index);
    void DisableEdge(size_t index);

    Edge&       operator[](size_t index);
    const Edge& operator[](size_t index) const;

    auto begin() { return iterator<false>{m_edges, m_indexes.cbegin()}; }
    auto end() { return iterator<false>{m_edges, m_indexes.cend()}; }

    auto begin() const { return iterator<true>{m_edges, m_indexes.cbegin()}; }
    auto end() const { return iterator<true>{m_edges, m_indexes.cend()}; }

    const std::vector<Edge>& Original() { return m_edges; }
    const std::vector<Edge>& Original() const { return m_edges; }
private:
    std::vector<Edge> m_edges{};
    std::list<size_t> m_indexes{};
};
} // namespace Graph::Details
