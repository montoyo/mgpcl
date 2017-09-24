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

#pragma once
#include "String.h"
#include <initializer_list>

namespace m
{
    
    class Version
    {
    public:
        Version()
        {
            m_data = nullptr;
            m_count = 0;
        }

        Version(const String &str)
        {
            m_data = nullptr;
            m_count = 0;
            parse(str);
        }

        Version(uint32_t a)
        {
            m_data = new uint32_t[1];
            m_count = 1;
            m_data[0] = a;
        }

        Version(uint32_t a, uint32_t b)
        {
            m_data = new uint32_t[2];
            m_count = 2;
            m_data[0] = a;
            m_data[1] = b;
        }

        Version(uint32_t a, uint32_t b, uint32_t c)
        {
            m_data = new uint32_t[3];
            m_count = 3;
            m_data[0] = a;
            m_data[1] = b;
            m_data[2] = c;
        }

        Version(std::initializer_list<uint32_t> il);

        Version(const Version &src)
        {
            m_count = src.m_count;

            if(m_count == 0)
                m_data = nullptr;
            else {
                m_data = new uint32_t[m_count];
                Mem::copy(m_data, src.m_data, sizeof(uint32_t) * m_count);
            }
        }

        Version(Version &&src)
        {
            m_data = src.m_data;
            m_count = src.m_count;
            src.m_data = nullptr;
        }

        ~Version()
        {
            if(m_data != nullptr)
                delete[] m_data;
        }

        bool parse(const String &str);
        String toString() const;
        Version &trim();
        Version trimmed() const;
        int hash() const;

        uint8_t count() const
        {
            return m_count;
        }

        uint32_t operator[] (uint8_t idx) const
        {
            return idx < m_count ? m_data[idx] : 0;
        }

        bool operator > (const Version &src) const;
        bool operator < (const Version &src) const;
        bool operator >= (const Version &src) const;
        bool operator <= (const Version &src) const;
        bool operator == (const Version &src) const;
        bool operator != (const Version &src) const;

        Version &operator = (const Version &src);
        Version &operator = (Version &&src)
        {
            if(m_data != nullptr)
                delete[] m_data;

            m_data = src.m_data;
            m_count = src.m_count;
            src.m_data = nullptr;
            return *this;
        }

    private:
        Version(const Version &src, uint8_t cnt)
        {
            if(cnt == 0) {
                m_data = nullptr;
                m_count = 0;
            } else {
                m_count = cnt;
                m_data = new uint32_t[cnt];
                Mem::copy(m_data, src.m_data, sizeof(uint32_t) * cnt);
            }
        }

        uint32_t *m_data;
        uint8_t m_count;
    };

}

