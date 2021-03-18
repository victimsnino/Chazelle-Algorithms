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

#include <Graph.h>

#include <algorithm>

namespace MST::Details
{
MSTSoftHeapDecorator::MSTSoftHeapDecorator(size_t r, size_t label_i, std::optional<size_t> label_j)
    : SoftHeapCpp<EdgePtrWrapper>{r}
    , m_label{label_i, label_j} {}

void MSTSoftHeapDecorator::Insert(EdgePtrWrapper new_key)
{
    new_key.SetLastHeapIndex(m_label);

    m_items.push_back(new_key);
    SoftHeapCpp<EdgePtrWrapper>::Insert(new_key);
}

EdgePtrWrapper MSTSoftHeapDecorator::DeleteMin()
{
    auto value = SoftHeapCpp<EdgePtrWrapper>::DeleteMin();
    m_items.remove(value);
    return value;
}

void MSTSoftHeapDecorator::Meld(MSTSoftHeapDecorator& other)
{
    std::ranges::for_each(other.m_items,
                          [&](EdgePtrWrapper& wrapper)
                          {
                              wrapper.SetLastHeapIndex(m_label);
                          });

    m_items.splice(m_items.end(), other.m_items);
    SoftHeapCpp<EdgePtrWrapper>::Meld(other);
}
} // namespace MST::Details
