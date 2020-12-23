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
#include <functional>
#include <list>
#include <memory>
#include <stdexcept>

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
    ItemType DeleteMin();

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
    struct ChildPerRank
    {
        size_t                          rank{};
        std::shared_ptr<Node<ItemType>> child{};
    };

public:
    Node(std::shared_ptr<ItemType>&& item, size_t r)
        : m_r(r)
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
        return (m_childs.empty() ? 0 : m_childs.front().rank);
    }

    bool Empty() const
    {
        return m_values.empty();
    }

    bool IsNeedRemeldChilds() const
    {
        return m_childs.size() < GetRank() / 2;
    }

    std::shared_ptr<ItemType> PopFront()
    {
        auto value_to_remove = std::move(m_values.front());
        m_values.pop_front();
        return value_to_remove;
    }

    /**************************** Modifiers ************************************/

    void Meld(std::shared_ptr<Node<ItemType>>&& another)
    {
        m_childs.insert(m_childs.begin(), { GetRank()+1, std::move(another) });
    }

    void ReMeld(std::function<void(std::shared_ptr<Head<ItemType>>&& new_head)>&& func_to_meld)
    {
        std::for_each(std::make_move_iterator(m_childs.begin()),
                      std::make_move_iterator(m_childs.end()),
                      [&](ChildPerRank&& child)
                      {
                          func_to_meld(std::make_shared<Head<ItemType>>(std::move(child.child)));
                      });

        m_childs.clear();
    }

    void Sift()
    {
        SiftDown(0);
    }
private:

    void SiftDown(int end)
    {
        auto begin = m_childs.size() - 2; // last element will be popped
        SiftImpl();

        for (int i = begin; i >= end; --i)
        {
            const auto& [rank, child] = m_childs[i];
            if(rank > m_r && (rank % 2 == 1 || child->GetRank() < rank - 1))
            {
                SiftDown(i+1);
            }
        }
    }

    void SiftImpl()
    {
        if (m_childs.empty())
            return;

        auto [last_rank, last_child] = m_childs.back();
        m_childs.pop_back();

        m_values.insert(m_values.begin(),
                        std::make_move_iterator(last_child->m_values.begin()),
                        std::make_move_iterator(last_child->m_values.end()));

        if (last_child->m_childs.empty())
            return;

        auto new_pos = m_childs.size();
        m_childs.insert(m_childs.end(),
                        std::make_move_iterator(last_child->m_childs.begin()),
                        std::make_move_iterator(last_child->m_childs.end()));

        m_childs[new_pos].rank = last_rank;
    }

private:
    std::list<std::shared_ptr<ItemType>> m_values{};
    std::vector<ChildPerRank>            m_childs{};
    const std::size_t                    m_r;
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

    const std::shared_ptr<Node<ItemType>>& GetRoot() const
    {
        return m_root;
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
    Meld(std::make_shared<THead>(std::make_shared<TNode>(std::make_shared<ItemType>(std::move(value)), m_r)));
}

template<typename ItemType>
ItemType SoftHeapCpp<ItemType>::DeleteMin()
{
    if (m_queues.empty())
        throw  std::out_of_range{"No available queues"};

    auto possible_candidate = m_queues.front()->GetSuffixMin().lock();
    while(possible_candidate->GetRoot()->Empty())
    {
        if (possible_candidate->GetRoot()->IsNeedRemeldChilds())
        {
            if (auto itr_to_fix = m_queues.erase(std::find(m_queues.begin(), m_queues.end(), possible_candidate));
                itr_to_fix != m_queues.begin())
                FixMinList(--itr_to_fix);
            possible_candidate->GetRoot()->ReMeld(std::bind(&SoftHeapCpp<ItemType>::Meld, this, std::placeholders::_1));
        }
        else
        {
            possible_candidate->GetRoot()->Sift();

            auto itr_to_fix_min_list = std::find(m_queues.begin(), m_queues.end(), possible_candidate);
            if (possible_candidate->GetRoot()->Empty())
            {
                itr_to_fix_min_list = m_queues.erase(itr_to_fix_min_list);
                if (itr_to_fix_min_list != m_queues.begin())
                    --itr_to_fix_min_list;
            }
            FixMinList(itr_to_fix_min_list);
        }
        possible_candidate = m_queues.front()->GetSuffixMin().lock();
    }

    return std::move(*(possible_candidate->GetRoot()->PopFront()));
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

        // We can insert it now
        if (next_queue_itr == m_queues.end() || (*next_queue_itr)->GetRank() != new_head->GetRank())
        {
            FixMinList(m_queues.emplace(next_queue_itr, std::move(new_head)));
            break;
        }

        (*next_queue_itr)->Meld(std::move(new_head));
        new_head = std::move(*next_queue_itr);

        // Now we need to extract it and to insert in the correct place
        m_queues.erase(next_queue_itr);
    }
}

template<typename ItemType>
void SoftHeapCpp<ItemType>:: FixMinList(typename std::list<THeadPtr>::iterator end)
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
