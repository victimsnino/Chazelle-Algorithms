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

#include <algorithm>

namespace MST::Details
{
MSTSoftHeapDecorator::MSTSoftHeapDecorator(size_t r)
    : m_heap{r,
             [](EdgePtrWrapperShared& item, const EdgePtrWrapperShared& ckey)
             {
                 item.shared_pointer->SetWorkingCost(ckey.shared_pointer->GetWorkingCost());
             }} {}

void MSTSoftHeapDecorator::Insert(EdgePtrWrapper new_key)
{
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
    auto                                 data = m_heap.ExtractItems();
    MSTSoftHeapDecorator::ExtractedItems to_out{};
    auto                                 convert = [](EdgePtrWrapperShared& shared)
    {
        return std::move(*shared.shared_pointer);
    };
    std::ranges::transform(data.corrupted, std::back_inserter(to_out.corrupted), convert);
    std::ranges::transform(data.items, std::back_inserter(to_out.items), convert);

    return to_out;
}
} // namespace MST::Details
