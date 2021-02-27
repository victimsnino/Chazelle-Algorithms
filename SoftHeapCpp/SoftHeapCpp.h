// MIT License
// 
// Copyright (c) 2020 Aleksey Loginov
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

#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <ranges>
#include <stdexcept>

template<typename ItemType>
class SoftHeapCpp
{
public:
    SoftHeapCpp(std::size_t r)
        : m_r(r) {}

    void Insert(ItemType value);
    ItemType DeleteMin();

private:
    struct Node
    {
        /*==================== CONSTRUCTORS =================*/
        Node(ItemType&& item, size_t r)
            : m_ckey{ std::move(item) }
            , m_values{ m_ckey }
            , m_r(r) {}

        Node(const Node& other) = delete;
        Node& operator=(const Node& other) = delete;

        Node(Node&& other) noexcept
            : m_rank{ other.m_rank }
            , m_ckey{ std::move(other.m_ckey) }
            , m_values{ std::move(other.m_values) }
            , m_childs{ std::move(other.m_childs) }
            , m_r{ other.m_r } {}

        Node& operator=(Node&& other) noexcept
        {
            if (this == &other)
                return *this;
            m_values = std::move(other.m_values);
            m_ckey = std::move(other.m_ckey);
            m_childs = std::move(other.m_childs);
            m_rank = other.m_rank;
            m_r = other.m_r;
            return *this;
        }

        /*==================== GETTERS ======================*/
        const ItemType&   GetCkey() const { return m_ckey; }
        size_t            GetRank() const { return m_rank; }
        size_t            GetChildsCount() const { return m_childs.size(); }
        bool              IsEmpty() const { return m_values.empty(); }

        std::list<Node>&& GetChilds() { return std::move(m_childs); }

        /*==================== MODIFIERS ====================*/
        void  Meld(Node&& node)
        {
            m_childs.emplace_front(std::move(node));
            ++m_rank;
        }

        void Sift()
        {
            SiftDown(m_childs.cbegin(), m_rank);
        }

        ItemType PopValue()
        {
            auto value = std::move(m_values.front());
            m_values.pop_front();
            return value;
        }
    private:
        void SiftDown(typename std::list<Node>::const_iterator current, size_t rank)
        {
            if (std::distance(current, m_childs.cend()) <= 1)
            {
                SiftImpl();
                return;
            }

            SiftDown(std::next(current), current->GetRank());

            if (rank > m_r && rank % 2 == 1)
                SiftDown(std::next(current), current->GetRank());
        }

        void SiftImpl()
        {
            if (m_childs.empty())
                return;

            auto last = std::move(m_childs.back());
            m_childs.pop_back();

            m_values.insert(m_values.begin(),
                std::make_move_iterator(last.m_values.begin()),
                std::make_move_iterator(last.m_values.end()));

            m_childs.insert(m_childs.end(),
                std::make_move_iterator(last.m_childs.begin()),
                std::make_move_iterator(last.m_childs.end()));

            m_ckey = std::move(last.m_ckey);
        }

    private:
        size_t              m_rank = 0;
        ItemType            m_ckey;
        std::list<ItemType> m_values;
        std::list<Node>     m_childs{}; // sorted is descending by ranks
        size_t              m_r = 0;
    };

    struct Head
    {
        /*==================== CONSTRUCTORS =================*/
        Head(Node&& node)
            : m_rank{ node.GetRank() }
            , m_root(std::move(node)){}

        Head(const Head& other)            = delete;
        Head& operator=(const Head& other) = delete;

        Head(Head&& other) noexcept
            : m_rank{other.m_rank}
            , m_root{std::move(other.m_root)}
            , m_suffix_min{other.m_suffix_min} {}

        Head& operator=(Head&& other) noexcept
        {
            if (this == &other)
                return *this;
            m_root       = std::move(other.m_root);
            m_suffix_min = other.m_suffix_min;
            m_rank       = other.m_rank;
            return *this;
        }

        /*==================== GETTERS ======================*/
        size_t          GetRank() const { return m_rank; }
        Head*           GetSuffixMin() const { return m_suffix_min; }
        const ItemType& GetCkey() const { return m_root.GetCkey(); }

        bool IsRootEmpty() const { return m_root.IsEmpty(); }
        bool IsNeedRemeldChilds() const { return m_root.GetChildsCount() < m_rank / 2; }

        /*==================== SETTERS ======================*/
        void  SetSuffixMin(Head* const suffix_mix) { m_suffix_min = suffix_mix; }

        /*==================== MODIFIERS ====================*/
        [[nodiscard]] Node&& Meld(Node&& new_root)
        {
            if (new_root.GetCkey() < m_root.GetCkey())
                std::swap(new_root, m_root);

            m_root.Meld(std::move(new_root));
            m_rank = m_root.GetRank();
            return std::move(m_root);
        }

        void Sift()
        {
            m_root.Sift();
        }

        std::list<Node>&& GetChildsAndDestroy()
        {
            return std::move(m_root.GetChilds());
        }

        ItemType PopValue()
        {
            return m_root.PopValue();
        }

    private:
        size_t m_rank;
        Node   m_root;
        Head*  m_suffix_min{};
    };

    void Meld(Node&& new_node);
    void FixMinList(typename std::list<Head>::iterator end);
private:
    const std::size_t m_r;
    std::list<Head>   m_queues{};
};

template<typename ItemType>
void SoftHeapCpp<ItemType>::Insert(ItemType value)
{
    Meld(Node{std::move(value), m_r});
}

template<typename ItemType>
ItemType SoftHeapCpp<ItemType>::DeleteMin()
{
    if (m_queues.empty())
        throw  std::out_of_range{"No available queues"};

    auto candidate_queue = m_queues.front().GetSuffixMin();
    while (candidate_queue->IsRootEmpty())
    {
        if (candidate_queue->IsNeedRemeldChilds())
        {
            auto itr        = std::ranges::find(m_queues, candidate_queue, [](Head& head) { return &head; });
            auto temp_queue = std::move(*candidate_queue);

            if (auto itr_to_fix = m_queues.erase(itr); itr_to_fix != m_queues.begin())
                FixMinList(--itr_to_fix);

            auto&& childs = temp_queue.GetChildsAndDestroy();
            std::for_each(std::make_move_iterator(childs.begin()),
                          std::make_move_iterator(childs.end()),
                          [&](Node&& node)
                          {
                              Meld(std::move(node));
                          });
        }
        else
        {
            candidate_queue->Sift();

            auto itr_to_fix_min_list = std::ranges::find(m_queues, candidate_queue, [](Head& head) { return &head; });
            if (candidate_queue->IsRootEmpty())
            {
                itr_to_fix_min_list = m_queues.erase(itr_to_fix_min_list);
                if (itr_to_fix_min_list != m_queues.begin())
                    --itr_to_fix_min_list;
            }
            if(!m_queues.empty())
                FixMinList(itr_to_fix_min_list);
        }
        candidate_queue = m_queues.front().GetSuffixMin();
    }

    return candidate_queue->PopValue();
}

template<typename ItemType>
void SoftHeapCpp<ItemType>::Meld(Node&& new_node)
{
    while (true)
    {
        auto new_rank = new_node.GetRank();

        auto next_queue_itr = std::ranges::lower_bound(m_queues, new_rank, std::ranges::less{}, &Head::GetRank);

        // We can insert it now
        if (next_queue_itr == m_queues.end() || next_queue_itr->GetRank() != new_rank)
        {
            FixMinList(m_queues.emplace(next_queue_itr, std::move(new_node)));
            break;
        }

        new_node = std::move(next_queue_itr->Meld(std::move(new_node)));

        // Now we need to extract it and insert in the correct place
        m_queues.erase(next_queue_itr);
    }
}

template<typename ItemType>
void SoftHeapCpp<ItemType>:: FixMinList(typename std::list<Head>::iterator end)
{
    Head* current_min_suffix = &*end;
    if (++end != m_queues.end())
        current_min_suffix = end->GetSuffixMin();

    std::for_each(std::make_reverse_iterator(end),
                  m_queues.rend(),
                  [&](Head& head)
                  {
                      if (head.GetCkey() < current_min_suffix->GetCkey())
                          current_min_suffix = &head;

                      head.SetSuffixMin(current_min_suffix);
                  });
}
