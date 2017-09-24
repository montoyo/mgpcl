/* Copyright (C) 2017 BARBOTIN Nicolas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "mgpcl/Version.h"

m::Version::Version(std::initializer_list<uint32_t> il)
{
    if(il.size() <= 0 || il.size() >= 256) {
        m_count = 0;
        m_data = nullptr;
    } else {
        m_count = static_cast<uint8_t>(il.size());
        m_data = new uint32_t[m_count];

        for(uint8_t i = 0; i < m_count; i++)
            m_data[i] = il.begin()[i];
    }
}

bool m::Version::parse(const String &str)
{
    int cnt = 0;
    for(int i = 0; i < str.length(); i++) {
        if(str[i] == '.')
            cnt++;
        else if(str[i] < '0' || str[i] > '9')
            return false;
    }

    if(cnt >= 255) //And not 256 because we increment it...
        return false;

    m_count = static_cast<uint8_t>(++cnt);
    m_data = new uint32_t[cnt];
    Mem::zero(m_data, sizeof(uint32_t) * cnt);

    uint8_t pos = 0;
    for(int i = 0; i < str.length(); i++) {
        if(str[i] == '.')
            pos++;
        else
            m_data[pos] = m_data[pos] * 10 + static_cast<uint32_t>(str[i] - '0');
    }

    return true;
}

m::String m::Version::toString() const
{
    if(m_count == 0)
        return String('0', 1);

    String ret(m_count * 2 - 1);
    bool first = true;

    for(uint8_t i = 0; i < m_count; i++) {
        if(first)
            first = false;
        else
            ret += '.';

        ret += String::fromUInteger(m_data[i]);
    }

    return ret;
}

m::Version &m::Version::trim()
{
    for(; m_count > 0; m_count--) {
        if(m_data[m_count - 1] != 0)
            break;
    }

    if(m_count == 0 && m_data != nullptr) {
        delete[] m_data;
        m_data = nullptr;
    }

    return *this;
}

m::Version m::Version::trimmed() const
{
    uint8_t i;
    for(i = m_count; i > 0; i--) {
        if(m_data[i - 1] != 0)
            break;
    }

    return Version(*this, i);
}

int m::Version::hash() const
{
    int ret = 0;
    int mul = 1;

    for(int i = m_count - 1; i >= 0; i--) {
        ret += static_cast<int>(m_data[i]) * mul;
        mul *= 31;
    }

    return ret;
}

bool m::Version::operator > (const Version &src) const
{
    uint8_t len = m_count > src.m_count ? m_count : src.m_count;
    for(uint8_t i = 0; i < len; i++) {
        if((*this)[i] != src[i])
            return (*this)[i] > src[i];
    }

    return false; //Equal
}

bool m::Version::operator < (const Version &src) const
{
    uint8_t len = m_count > src.m_count ? m_count : src.m_count;
    for(uint8_t i = 0; i < len; i++) {
        if((*this)[i] != src[i])
            return (*this)[i] < src[i];
    }

    return false; //Equal
}

bool m::Version::operator >= (const Version &src) const
{
    uint8_t len = m_count > src.m_count ? m_count : src.m_count;
    for(uint8_t i = 0; i < len; i++) {
        if((*this)[i] != src[i])
            return (*this)[i] > src[i];
    }

    return true; //Equal
}

bool m::Version::operator <= (const Version &src) const
{
    uint8_t len = m_count > src.m_count ? m_count : src.m_count;
    for(uint8_t i = 0; i < len; i++) {
        if((*this)[i] != src[i])
            return (*this)[i] < src[i];
    }

    return true; //Equal
}

bool m::Version::operator == (const Version &src) const
{
    uint8_t len = m_count > src.m_count ? m_count : src.m_count;
    for(uint8_t i = 0; i < len; i++) {
        if((*this)[i] != src[i])
            return false;
    }

    return true;
}

bool m::Version::operator != (const Version &src) const
{
    uint8_t len = m_count > src.m_count ? m_count : src.m_count;
    for(uint8_t i = 0; i < len; i++) {
        if((*this)[i] != src[i])
            return true;
    }

    return false;
}

m::Version &m::Version::operator = (const Version &src)
{
    if(m_data == src.m_data)
        return *this;

    if(m_data != nullptr)
        delete[] m_data;

    m_count = src.m_count;

    if(m_count == 0)
        m_data = nullptr;
    else {
        m_data = new uint32_t[m_count];
        Mem::copy(m_data, src.m_data, sizeof(uint32_t) * m_count);
    }

    return *this;
}
