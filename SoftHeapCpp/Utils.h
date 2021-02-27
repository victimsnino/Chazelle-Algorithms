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

namespace Utils
{
template<typename CoreType>
class ComparableObject
{
public:
    ComparableObject(const CoreType* value)
        : m_value(value) {}

    ComparableObject(ComparableObject&& other) noexcept
        : m_value{other.m_value} {}

    ComparableObject& operator=(ComparableObject&& other) noexcept
    {
        if (this == &other)
            return *this;
        m_value = other.m_value;
        return *this;
    }

    bool operator==(const ComparableObject& rhs) const
    {
        if (m_value == rhs.m_value)
            return true;

        if (!m_value || !rhs.m_value)
            return false;

        return *m_value == *rhs.m_value;
    }

    bool operator<(const ComparableObject& rhs) const
    {
        if (!m_value) // aka infinity < const_value
            return false;

        if (!rhs.m_value) // aka const_value < infinity
            return true;

        return *m_value < *rhs.m_value;
    }

    bool operator!=(const ComparableObject& rhs) const { return !(*this == rhs); }

    bool operator<=(const ComparableObject& rhs) const { return !(rhs < *this); }
    bool operator>(const ComparableObject& rhs) const { return rhs < *this; }
    bool operator>=(const ComparableObject& rhs) const { return !(*this < rhs); }
private:
    const CoreType* m_value;
};
}
