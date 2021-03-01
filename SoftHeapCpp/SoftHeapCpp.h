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

// This source is the original source from the paper with small modifications

#pragma once

#include "Utils.h"

#include <list>
#include <memory>
#include <cassert>
#include <functional>

struct Head;
struct Node;

template<typename ItemType>
class SoftHeapCpp
{
public:
    SoftHeapCpp(size_t r);

    void Insert(ItemType new_key);
    ItemType DeleteMin();

private:
    struct Node
    {
        Node(ItemType item)
            : m_ckey{std::make_shared<ItemType>(item)}
            , m_rank{0}
        {
            m_values = std::make_shared<std::list<ItemType>>();
            m_values->push_front(std::move(item));
        }

        Node(std::unique_ptr<Node> top, std::unique_ptr<Node> bottom)
            : m_ckey{ top->m_ckey }
            , m_rank{ top->m_rank + 1 }
            , m_next{ std::move(top) }
            , m_child{ std::move(bottom) }
            , m_values{ m_next->m_values } { }

        size_t                                          GetRank() const { return m_rank; }
        [[nodiscard]] Utils::ComparableObject<ItemType> GetCkey() const { return {m_ckey.get()}; }
        bool                                            IsInfntyCkey() const { return !m_ckey; }
        [[nodiscard]] std::unique_ptr<Node>             ExtractChild() { return std::move(m_child); }


        bool        IsNoValues() const { return !m_values || m_values->empty(); }

        void Sift(size_t r);

        void ForEachNodeWithChildOnLevel(std::function<void(Node* node)> func)
        {
            if (m_next)
            {
                func(this);
                m_next->ForEachNodeWithChildOnLevel(std::move(func));
            }
        }

        ItemType PopValue()
        {
            assert(!!m_values && !m_values->empty());
            auto value = std::move(m_values->front());
            m_values->pop_front();
            return value;
        }
    private:
        std::shared_ptr<ItemType>            m_ckey;
        const size_t                         m_rank;
        std::unique_ptr<Node>                m_next{};
        std::unique_ptr<Node>                m_child{};
        std::shared_ptr<std::list<ItemType>> m_values{};
    };

    struct Head
    {
        std::unique_ptr<Node> queue{};
        struct Head *         next{}, *prev{}, *suffix_min{};
        size_t                rank{};
    };

    void  Meld(std::unique_ptr<Node> q);
    void  FixMinlist(Head* h);
private:
    Head*        m_header{};
    Head*        m_tail{};
    const size_t m_r;
};

template<typename ItemType>
SoftHeapCpp<ItemType>::SoftHeapCpp<ItemType>(size_t r)
    : m_r(r)
{
    m_header = new Head();
    m_tail = new Head();

    m_tail->rank = std::numeric_limits<size_t>::max();
    m_header->next = m_tail;
    m_tail->prev = m_header;
}

template<typename ItemType>
void SoftHeapCpp<ItemType>::Insert(ItemType new_key)
{
    Meld(std::make_unique<Node>(std::move(new_key)));
}

template<typename ItemType>
ItemType SoftHeapCpp<ItemType>::DeleteMin()
{
    assert(m_header->next);

    Head* h = m_header->next->suffix_min;
    assert(h);

    while (h->queue->IsNoValues())
    {
        
        size_t child_count = 0;
        h->queue->ForEachNodeWithChildOnLevel([&](Node* node) {child_count += 1; });

        // remeld childs
        if (child_count < h->rank / 2)
        {
            h->prev->next = h->next;
            h->next->prev = h->prev;
            FixMinlist(h->prev);

            h->queue->ForEachNodeWithChildOnLevel([&](Node* node)
            {
                Meld(node->ExtractChild());
            });
        }
        else
        {
            h->queue->Sift(m_r);
            if (h->queue->IsInfntyCkey())
            {
                h->prev->next = h->next;
                h->next->prev = h->prev;
                h = h->prev;
            }
            FixMinlist(h);
        }
        h = m_header->next->suffix_min;
    } /* end of outer while loop */

    return h->queue->PopValue();
}

template<typename ItemType>
void SoftHeapCpp<ItemType>::Meld(std::unique_ptr<Node> q)
{
    Head* tohead = m_header->next;
    while (q->GetRank() > tohead->rank)
        tohead = tohead->next;

    Head* prevhead = tohead->prev;

    while (q->GetRank() == tohead->rank)
    {
        std::unique_ptr<Node> top;
        std::unique_ptr<Node> bottom;
        if (tohead->queue->GetCkey() > q->GetCkey())
        {
            top    = std::move(q);
            bottom = std::move(tohead->queue);
        }
        else
        {
            top    = std::move(tohead->queue);
            bottom = std::move(q);
        }
        q      = std::make_unique<Node>(std::move(top), std::move(bottom));
        tohead = tohead->next;
    }

    Head* h;
    if (prevhead == tohead->prev)
        h = new Head();
    else // actually we've moved out queue from this one...
        h = prevhead->next;
    h->queue       = std::move(q);
    h->rank        = h->queue->GetRank();
    h->prev        = prevhead;
    h->next        = tohead;
    prevhead->next = h;
    tohead->prev   = h;

    FixMinlist(h);
}

template<typename ItemType>
void SoftHeapCpp<ItemType>::FixMinlist(Head* h)
{
    Head* tmpmin;
    if (h->next == m_tail)
        tmpmin = h;
    else
        tmpmin = h->next->suffix_min;
    while (h != m_header)
    {
        if (h->queue->GetCkey() < tmpmin->queue->GetCkey())
            tmpmin = h;
        h->suffix_min = tmpmin;
        h = h->prev;
    }
}

template<typename ItemType>
void SoftHeapCpp<ItemType>::Node::Sift(const size_t r)
{
    m_values.reset();
    if (!m_next && !m_child)
    {
        m_ckey.reset();
        return;
    }

    m_next->Sift(r);

    if (m_next->GetCkey() > m_child->GetCkey())
        std::swap(m_child, m_next);

    m_values = m_next->m_values;
    m_ckey   = m_next->m_ckey;

    // Sometimes we can do it twice due branching
    if (GetRank() > r &&
        (GetRank() % 2 == 1 || m_child->GetRank() < GetRank() - 1))
    {
        m_next->Sift(r);

        if (m_next->GetCkey() > m_child->GetCkey())
            std::swap(m_child, m_next);

        // Concatenate lists if not empty
        if (!m_next->IsInfntyCkey() && !m_next->m_values->empty())
        {
            m_values->insert(m_values->begin(),
                             std::make_move_iterator(m_next->m_values->begin()),
                             std::make_move_iterator(m_next->m_values->end()));
            m_next->m_values->clear();
            m_next->m_values.reset();
            m_ckey = std::move(m_next->m_ckey);
        }
    } /*  end of second sift */

    // Clean Up
    if (!m_child->IsInfntyCkey())
        return;

    if (m_next->IsInfntyCkey())
    {
        m_child.reset();
        m_next.reset();
    }
    else
    {
        m_child = std::move(m_next->m_child);
        m_next  = std::move(m_next->m_next);
    }
}
