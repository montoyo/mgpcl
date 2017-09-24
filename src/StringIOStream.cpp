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

#include "mgpcl/StringIOStream.h"

int m::StringOStream::write(const uint8_t *src, int sz)
{
    mDebugAssert(sz >= 0, "cannot write negative amount of bytes");

    if(m_pos >= m_data.length()) {
        m_data.append(reinterpret_cast<const char*>(src), sz);
        m_pos += sz;
    } else if(m_insert) {
        m_data.insert(m_pos, reinterpret_cast<const char*>(src), sz);
        m_pos += sz;
    } else {
        int rep = m_data.length() - m_pos;
        if(rep > sz)
            rep = sz;

        Mem::copy(m_data.begin(), src, rep);
        sz -= rep;

        if(sz > 0)
            m_data.append(reinterpret_cast<const char*>(src + rep), sz);
    }

    return sz;
}

bool m::StringOStream::seek(int amount, SeekPos sp)
{
    switch(sp) {
    case SeekPos::Beginning:
        mDebugAssert(amount < 0, "cannot seek negative amount from beginning");
        m_pos = amount;
        break;

    case SeekPos::Relative:
        if(amount == 0)
            return true;
        else if(amount > 0)
            m_pos += amount;
        else {
            //Negative
            amount = -amount;
            if(amount > m_pos)
                return false;

            m_pos -= amount;
        }
        break;

    case SeekPos::End:
        if(amount >= 0)
            m_pos = m_data.length() + amount;
        else {
            //Negative
            amount = -amount;
            if(amount > m_data.length())
                return false;

            m_pos = m_data.length() - amount;
        }

        break;

    default:
        return false;
    }

    if(m_pos > m_data.length())
        m_data.append(m_fill, m_pos - m_data.length());

    return true;
}

int m::StringIStream::read(uint8_t *dst, int sz)
{
    mDebugAssert(sz >= 0, "cannot read a negative amount of bytes");
    int rem = m_data.length() - m_pos;
    if(rem > sz)
        rem = sz;

    Mem::copy(dst, m_data.begin() + m_pos, rem);
    m_pos += rem;
    return rem;
}

bool m::StringIStream::seek(int amount, SeekPos sp)
{
    switch(sp) {
    case SeekPos::Beginning:
        mDebugAssert(amount >= 0, "cannot seek backwards from the beginning");
        if(amount > m_data.length())
            return false;

        m_pos = amount;
        return true;

    case SeekPos::Relative:
        if(amount >= 0) {
            if(m_pos + amount > m_data.length())
                return false;

            m_pos += amount;
        } else {
            amount = -amount;
            if(amount > m_pos)
                return false;

            m_pos -= amount;
        }

        return true;

    case SeekPos::End:
        if(amount > 0)
            return false;
        
        amount = -amount;
        if(amount > m_data.length())
            return false;

        m_pos = m_data.length() - amount;
        return true;
    }

    return false;
}
