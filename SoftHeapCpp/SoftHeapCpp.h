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

#include "Utils.h"

#include <algorithm>
#include <list>
#include <memory>

namespace Details
{
template<typename ItemType>
class Node;

template<typename ItemType>
class Head;
}

template<typename ItemType>
class SoftHeapCpp
{
public:
    SoftHeapCpp(std::size_t r)
        : m_r(r) {}

    void Insert(ItemType value);

private:
    using TNode = Details::Node<ItemType>;
    using TNodePtr = std::shared_ptr<TNode>;

    using THead = Details::Head<ItemType>;
    using THeadPtr = std::shared_ptr<THead>;

    void Meld(THeadPtr&& new_head);
    void FixMinList(typename std::list<THeadPtr>::iterator end);
private:
    const std::size_t   m_r;
    std::list<THeadPtr> m_queues{};
};

namespace Details
{
template<typename ItemType>
class Node
{
public:
    Node(std::shared_ptr<ItemType>&& item)
    {
        m_values.emplace_front(std::move(item));
    }

    /**************************** Getters ************************************/
    Utils::ComparableObject<ItemType> Ckey() const
    {
        return Utils::ComparableObject<ItemType>{m_values.empty() ? nullptr : m_values.front().get()};
    }

    std::size_t GetRank() const
    {
        return m_rank;
    }


    /**************************** Modifiers ************************************/

    void Meld(std::shared_ptr<Node<ItemType>>&& another)
    {
        ++m_rank;
        m_childs.emplace_front(std::move(another));
    }

private:
    std::list<std::shared_ptr<ItemType>>       m_values{};
    std::list<std::shared_ptr<Node<ItemType>>> m_childs{};
    std::size_t                                m_rank = 0;
};

template<typename ItemType>
class Head
{
public:
    Head(std::shared_ptr<Node<ItemType>>&& item)
        : m_root(std::move(item)) {}

    /**************************** Getters ************************************/

    std::size_t GetRank() const
    {
        return m_root->GetRank();
    }

    Utils::ComparableObject<ItemType> Ckey() const
    {
        return m_root->Ckey();
    }

    const std::weak_ptr<Head<ItemType>>& GetSuffixMin() const
    {
        return m_suffix_min;
    }

    /**************************** Modifiers ************************************/

    void Meld(std::shared_ptr<Head<ItemType>>&& another_head)
    {
        if (Ckey() >= another_head->Ckey())
            std::swap(m_root, another_head.get()->m_root);

        m_root->Meld(std::move(another_head->m_root));
    }

    void SetSuffixMin(const std::weak_ptr<Head<ItemType>>& suffix_min)
    {
        m_suffix_min = suffix_min;
    }

private:
    std::shared_ptr<Node<ItemType>> m_root{};
    std::weak_ptr<Head<ItemType>>   m_suffix_min{};
};
} // namespace Details

template<typename ItemType>
void SoftHeapCpp<ItemType>::Insert(ItemType value)
{
    Meld(std::make_shared<THead>(std::make_shared<TNode>(std::make_shared<ItemType>(value))));
}

template<typename ItemType>
void SoftHeapCpp<ItemType>::Meld(THeadPtr&& new_head)
{
    while (true)
    {
        const auto next_queue_itr = std::find_if(m_queues.begin(),
                                                 m_queues.end(),
                                                 [rank = new_head->GetRank()](const THeadPtr& queue)
                                                 {
                                                     return rank <= queue->GetRank();
                                                 });

        if (next_queue_itr == m_queues.end() || (*next_queue_itr)->GetRank() != new_head->GetRank())
        {
            FixMinList(m_queues.emplace(next_queue_itr, std::move(new_head)));
            break;
        }

        auto head_from_heap_to_meld_with = std::move(*next_queue_itr);
        m_queues.erase(next_queue_itr);

        head_from_heap_to_meld_with->Meld(std::move(new_head));
        new_head = head_from_heap_to_meld_with;
    }
}

template<typename ItemType>
void SoftHeapCpp<ItemType>::FixMinList(typename std::list<THeadPtr>::iterator end)
{
    std::shared_ptr<THead> current_min_suffix = *end;
    if (++end != m_queues.end())
        current_min_suffix = (*end)->GetSuffixMin().lock();

    std::for_each(std::make_reverse_iterator(end),
                  m_queues.rend(),
                  [&](THeadPtr& ptr)
                  {
                      if (ptr->Ckey() < current_min_suffix->Ckey())
                          current_min_suffix = ptr;

                      ptr->SetSuffixMin(current_min_suffix);
                  });
}
