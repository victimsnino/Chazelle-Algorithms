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
#include <array>
#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace Graph
{
namespace Details
{
    class Edge
    {
    public:
        Edge(size_t i, size_t j, uint32_t weight);

        auto GetVertexes() const { return std::tie(m_i, m_j); }

        void SetIsContracted() { m_is_contracted = true; }
        bool IsContracted() const { return m_is_contracted; }

        bool operator<(const Edge& rhs) const { return m_weight < rhs.m_weight; }
        //bool operator>(const Edge& rhs) const { return rhs < *this; }
    private:
        const size_t   m_i;
        const size_t   m_j;
        const uint32_t m_weight;
        /**
         * \brief Set true if current edge was contracted by ContractEdge function. We don't need to check this edge in future
         */
        bool m_is_contracted{false};
        /**
         * \brief Set true if current edge is "multiple edge"and some else edge has lower weight
         */
        bool m_is_disabled{false};
    };

    class MemberOfSubGraph
    {
    public:
        MemberOfSubGraph(size_t parent, size_t rank = 0);

        size_t GetParent() const { return m_parent; }
        void   SetParent(size_t parent) { m_parent = parent; }

        bool IsRoot() const { return m_parent == m_original_vertex; }

        // Equal or not by parents (AKA same tree or not)
        bool operator==(const MemberOfSubGraph& rhs) const{return m_parent == rhs.m_parent;}
        bool operator!=(const MemberOfSubGraph& rhs) const { return !(*this == rhs); }

        // Compare by weights
        bool operator<(const MemberOfSubGraph& rhs) const{return m_rank < rhs.m_rank;}
        //bool operator>(const MemberOfSubGraph& rhs) const { return rhs < *this; }
    private:
        size_t m_parent{};
        const size_t m_original_vertex;
        size_t m_rank{}; // unused in case of m_parent != i
    };

    struct ActiveEdgesView
    {
        ActiveEdgesView(std::vector<Details::Edge>& edges)
            : m_edges(edges) {}

        struct iterator
        {
            iterator(std::vector<Details::Edge>& edges, std::list<size_t>::const_iterator itr)
                : m_edges(edges)
                , m_itr(std::move(itr)) {}

            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = Edge;
            using pointer = Edge*;  
            using reference = Edge&; 

            void  operator ++() { ++m_itr; }
            bool  operator !=(const iterator& other) const { return m_itr != other.m_itr; }
            reference operator *() const { return m_edges[*m_itr]; }
        private:
            std::vector<Details::Edge>&       m_edges;
            std::list<size_t>::const_iterator m_itr;
        };

        void EdgeAdded();
        void ContractEdge(size_t index) { m_indexes.remove(index); }

        auto begin() const { return iterator{m_edges, m_indexes.cbegin()}; }
        auto end() const { return iterator{m_edges, m_indexes.cend()}; }
    private:
        std::vector<Details::Edge>& m_edges;
        std::list<size_t>           m_indexes{};
    };
}

class Graph
{
public:
    template<size_t size, typename Type>
    Graph(const std::array<std::array<Type, size>, size>& adjacency);

    Graph() = default;

    void BoruvkaPhase();
    void ContractEdge(size_t edge_index);

    size_t GetVertexesCount()const;
    size_t GetEdgesCount() const;
    std::vector<std::tuple<size_t, size_t>> GetMST() const;

private:
    void   AddEdge(size_t begin, size_t end, uint32_t weight);
    size_t FindRootOfSubGraph(size_t i);
    auto& Edges() { return m_edges_view; }
    auto& Edges() const { return m_edges_view; }
private:
    std::vector<Details::Edge>             m_edges{};
    Details::ActiveEdgesView               m_edges_view{ m_edges };
    std::vector<Details::MemberOfSubGraph> m_subgraphs{}; // aka vertexes
};

template<size_t size, typename Type>
Graph::Graph(const std::array<std::array<Type, size>, size>& adjacency)
{
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < i; ++j)
            AddEdge(i, j, std::max(adjacency[i][j], adjacency[j][i]));

        m_subgraphs.emplace_back(i);
    }
}

void ToFile(const Graph& graph, const std::string& graph_name, bool show = false, bool with_mst = false);
} // namespace Graph
