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

#include <cassert>
#include <functional>
#include <list>
#include <memory>

struct Head;
struct Node;

template<typename ItemType>
class SoftHeapCpp
{
public:
    SoftHeapCpp(size_t r);
    virtual          ~SoftHeapCpp() = default;

    virtual void     Insert(ItemType new_key);
    virtual ItemType DeleteMin();

    virtual void Meld(SoftHeapCpp& other);

    struct ExtractedItems
    {
        std::list<ItemType> corrupted{};
        std::list<ItemType> items{};
    };

    virtual ExtractedItems ExtractItems();
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
            : m_ckey{top->m_ckey}
            , m_rank{top->m_rank + 1}
            , m_next{std::move(top)}
            , m_child{std::move(bottom)}
            , m_values{m_next->m_values} { }

        size_t                                          GetRank() const { return m_rank; }
        [[nodiscard]] Utils::ComparableObject<ItemType> GetCkey() const { return {m_ckey.get()}; }
        bool                                            IsInfntyCkey() const { return !m_ckey; }
        [[nodiscard]] std::unique_ptr<Node>             ExtractChild() { return std::move(m_child); }


        bool IsNoValues() const { return !m_values || m_values->empty(); }

        void Sift(size_t r);

        void ForEachNodeWithChildOnLevel(std::function<void(Node* node)> func)
        {
            if (m_next)
            {
                func(this);
                m_next->ForEachNodeWithChildOnLevel(std::move(func));
            }
        }

        void ExtractCorruptedItems(ExtractedItems& result)
        {
            if (m_values || !m_ckey)
            {
                for (auto it = m_values->begin(); it != m_values->end();)
                {
                    auto& list_to_insert = (*it < *m_ckey) ? result.corrupted : result.items;

                    auto value_to_extract = it++;
                    list_to_insert.splice(list_to_insert.end(), *m_values, value_to_extract);
                }
            }

            if (m_next)
                m_next->ExtractCorruptedItems(result);

            if (m_child)
                m_child->ExtractCorruptedItems(result);
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
        Head(size_t rank, std::unique_ptr<Node> queue = {})
            : m_queue{std::move(queue)}
            , m_rank{rank} {}

        std::unique_ptr<Node> ExtractQueue() { return std::move(m_queue); }

        const std::unique_ptr<Node>& GetQueue() const { return m_queue; }
        const std::shared_ptr<Head>& GetNext() const { return m_next; }
        const std::shared_ptr<Head>& GetPrev() const { return m_prev; }
        const std::shared_ptr<Head>& GetSuffixMin() const { return m_suffix_min; }
        size_t                       GetRank() const { return m_rank; }

        void SetNext(const std::shared_ptr<Head>& next) { m_next = next; }
        void SetPrev(const std::shared_ptr<Head>& prev) { m_prev = prev; }
        void SetSuffixMin(const std::shared_ptr<Head>& suffix_min) { m_suffix_min = suffix_min; }
    private:
        std::unique_ptr<Node> m_queue{};
        std::shared_ptr<Head> m_next{};
        std::shared_ptr<Head> m_prev{};
        std::shared_ptr<Head> m_suffix_min{};
        const size_t          m_rank{};
    };

    void Meld(std::unique_ptr<Node> q);
    void FixMinlist(std::shared_ptr<Head> h);
private:
    std::shared_ptr<Head> m_header{};
    std::shared_ptr<Head> m_tail{};
    const size_t          m_r;
};

template<typename ItemType>
SoftHeapCpp<ItemType>::SoftHeapCpp(size_t r)
    : m_header{std::make_shared<Head>(0)}
    , m_tail{std::make_shared<Head>(std::numeric_limits<size_t>::max())}
    , m_r(r)
{
    m_header->SetNext(m_tail);
    m_tail->SetPrev(m_header);
}

template<typename ItemType>
void SoftHeapCpp<ItemType>::Insert(ItemType new_key)
{
    Meld(std::make_unique<Node>(std::move(new_key)));
}

template<typename ItemType>
ItemType SoftHeapCpp<ItemType>::DeleteMin()
{
    assert(m_header->GetNext());

    auto h = m_header->GetNext()->GetSuffixMin();
    assert(h);

    while (h->GetQueue()->IsNoValues())
    {
        size_t                                               child_count = 0;
        h->GetQueue()->ForEachNodeWithChildOnLevel([&](Node* node) { child_count += 1; });

        // remeld childs
        if (child_count < h->GetRank() / 2)
        {
            h->GetPrev()->SetNext(h->GetNext());
            h->GetNext()->SetPrev(h->GetPrev());
            FixMinlist(h->GetPrev());

            h->GetQueue()->ForEachNodeWithChildOnLevel([&](Node* node)
            {
                Meld(node->ExtractChild());
            });
        }
        else
        {
            h->GetQueue()->Sift(m_r);
            if (h->GetQueue()->IsInfntyCkey())
            {
                h->GetPrev()->SetNext(h->GetNext());
                h->GetNext()->SetPrev(h->GetPrev());
                h = h->GetPrev();
            }
            FixMinlist(h);
        }
        h = m_header->GetNext()->GetSuffixMin();
    } /* end of outer while loop */

    return h->GetQueue()->PopValue();
}

template<typename ItemType>
void SoftHeapCpp<ItemType>::Meld(SoftHeapCpp& other)
{
    auto           h = other.m_header->GetNext();

    while (h != other.m_tail)
    {
        Meld(h->ExtractQueue());
        h = h->GetNext();
    }
}

template<typename ItemType>
typename SoftHeapCpp<ItemType>::ExtractedItems SoftHeapCpp<ItemType>::ExtractItems()
{
    ExtractedItems result{};
    auto           h = m_header->GetNext();

    while (h != m_tail)
    {
        h->GetQueue()->ExtractCorruptedItems(result);
        h = h->GetNext();
    }

    result.corrupted.unique();
    result.items.unique();
    return result;
}

template<typename ItemType>
void SoftHeapCpp<ItemType>::Meld(std::unique_ptr<Node> q)
{
    auto tohead = m_header->GetNext();
    while (q->GetRank() > tohead->GetRank())
        tohead = tohead->GetNext();

    auto prevhead = tohead->GetPrev();

    while (q->GetRank() == tohead->GetRank())
    {
        std::unique_ptr<Node> top;
        std::unique_ptr<Node> bottom;
        if (tohead->GetQueue()->GetCkey() > q->GetCkey())
        {
            top    = std::move(q);
            bottom = std::move(tohead->ExtractQueue());
        }
        else
        {
            top    = std::move(tohead->ExtractQueue());
            bottom = std::move(q);
        }
        q      = std::make_unique<Node>(std::move(top), std::move(bottom));
        tohead = tohead->GetNext();
    }

    auto h = std::make_shared<Head>(q->GetRank(), std::move(q));
    h->SetPrev(prevhead);
    h->SetNext(tohead);
    prevhead->SetNext(h);
    tohead->SetPrev(h);

    FixMinlist(h);
}

template<typename ItemType>
void SoftHeapCpp<ItemType>::FixMinlist(std::shared_ptr<Head> h)
{
    std::shared_ptr<Head> tmpmin;
    if (h->GetNext() == m_tail)
        tmpmin = h;
    else
        tmpmin = h->GetNext()->GetSuffixMin();
    while (h != m_header)
    {
        if (h->GetQueue()->GetCkey() < tmpmin->GetQueue()->GetCkey())
            tmpmin = h;
        h->SetSuffixMin(tmpmin);
        h = h->GetPrev();
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
            m_ckey = m_next->m_ckey;
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
