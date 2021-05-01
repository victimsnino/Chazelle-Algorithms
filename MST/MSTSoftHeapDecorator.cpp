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

#include "MSTSoftHeapDecorator.h"

#include <Common.h>
#include <Graph.h>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <spdlog/spdlog.h>

#include <algorithm>

namespace MST::Details
{
MSTSoftHeapDecorator::MSTSoftHeapDecorator(size_t r, std::set<size_t>& bad_edges)
    : m_heap{r,
             [&bad_edges](EdgePtrWrapperShared& item, const EdgePtrWrapperShared& ckey)
             {
                 SPDLOG_DEBUG("SetWorking cost for {} cost {}", item.shared_pointer->GetEdge().GetIndex(), ckey.shared_pointer->GetWorkingCost());
                 if (item.shared_pointer->GetWorkingCost() < ckey.shared_pointer->GetWorkingCost())
                 {
                     SPDLOG_DEBUG("{} becomes corrupted", item.shared_pointer->GetEdge().GetIndex());
                     item.shared_pointer->SetIsCorrupted(true);
                     bad_edges.emplace(item.shared_pointer->GetEdge().GetIndex());
                 }
                 item.shared_pointer->SetWorkingCost(ckey.shared_pointer->GetWorkingCost());
             }} {}

void MSTSoftHeapDecorator::Insert(EdgePtrWrapper new_key)
{
    SPDLOG_DEBUG("New edge {}", new_key->GetIndex());
    auto ptr = std::make_shared<EdgePtrWrapper>(std::move(new_key));

    m_heap.Insert(EdgePtrWrapperShared{ptr});
    m_items.emplace_back(std::move(ptr));
}

EdgePtrWrapper MSTSoftHeapDecorator::DeleteMin()
{
    const auto value = m_heap.DeleteMin();
    auto       ptr   = value.shared_pointer;

    if (!Utils::IsRangeContains(m_items, value.shared_pointer))
        return DeleteMin();

    SPDLOG_DEBUG("Remove edge {}", ptr->GetEdge().GetIndex());
    m_items.remove(ptr);
    return *ptr;
}

EdgePtrWrapper* MSTSoftHeapDecorator::FindMin()
{
    const auto value_ptr = m_heap.FindMin();
    if (!value_ptr)
        return {};

    if (Utils::IsRangeContains(m_items, value_ptr->shared_pointer))
        return value_ptr->shared_pointer.get();

    m_heap.DeleteMin();
    return FindMin();
}

std::list<EdgePtrWrapper> MSTSoftHeapDecorator::DeleteAndReturnIf(std::function<bool(const EdgePtrWrapper& edge)> func)
{
    std::list<EdgePtrWrapper> result{};
    for (auto itr = m_items.begin(); itr != m_items.end();)
    {
        if (func(**itr))
        {
            SPDLOG_DEBUG("Delete edge {}", (*itr)->GetEdge().GetIndex());
            result.emplace_back(**itr);
            itr = m_items.erase(itr);
        }
        else
            ++itr;
    }
    return result;
}

void MSTSoftHeapDecorator::Meld(MSTSoftHeapDecorator& other)
{
    m_items.splice(m_items.end(), other.m_items);
    m_heap.Meld(other.m_heap);
}

MSTSoftHeapDecorator::ExtractedItems MSTSoftHeapDecorator::ExtractItems()
{
    ExtractedItems to_out{};

    for (auto& edge : m_items)
    {
       if (edge->GetIsCorrupted())
            to_out.corrupted.push_back(*edge);
        else
            to_out.items.push_back(*edge);
    }

    for (auto& edge : to_out.corrupted)
        SPDLOG_DEBUG("Corrupted edge {} original {} current {}", edge->GetIndex(), edge->GetWeight(), edge.GetWorkingCost());
    for (auto& edge : to_out.items)
        SPDLOG_DEBUG("Normal edge {} original {} current {}", edge->GetIndex(), edge->GetWeight(), edge.GetWorkingCost());
    m_items.clear();
    return to_out;
}
} // namespace MST::Details
