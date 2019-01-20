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
#include "Config.h"
#include "List.h"
#include "Util.h"
#include "Assert.h"
#include "VAList.h"
#include <functional>
#include <cstdlib>
#include <cctype>
#include <cwchar>
#include <limits>
#include <cmath>

namespace m
{
    template<typename T> class MGPCL_PREFIX TString
    {
    public:
        typedef T *Iterator;
        typedef const T *CIterator;

        static bool isBlankChar(T chr)
        {
            return chr == static_cast<T>(' ') || chr == static_cast<T>('\n') || chr == static_cast<T>('\r') || chr == static_cast<T>('\t') || chr == 0;
        }

        TString()
        {
            m_data    = new T[1];
            m_data[0] = 0;
            m_len     = 0;
            m_alloc   = 1;
        }

        TString(const TString<T> &src)
        {
            m_len   = src.m_len;
            m_alloc = src.m_alloc;
            m_data  = new T[m_alloc];
            mem::copy(m_data, src.m_data, (m_len + 1) * sizeof(T));
        }

        TString(TString<T> &&src)
        {
            m_len   = src.m_len;
            m_alloc = src.m_alloc;
            m_data  = src.m_data;

            src.m_data = nullptr;
        }

        TString(const T *str)
        {
            mDebugAssert(str != nullptr, "passed nullptr to string function");
            m_len = 0; //Can't use strlen() for this since it can also be a wchar_t!
            while(str[m_len] != 0)
                m_len++;

            m_alloc = m_len + 1;
            m_data = new T[m_alloc];
            mem::copy(m_data, str, (m_len + 1) * sizeof(T));
        }

        TString(const T *str, int len)
        {
            mDebugAssert(str != nullptr, "passed nullptr to string function");

            if(len < 0) {
                m_len = 0; //Can't use strlen() for this since it can also be a wchar_t!
                while(str[m_len] != 0)
                    m_len++;
            } else
                m_len = len;

            m_alloc = m_len + 1;
            m_data  = new T[m_alloc];
            mem::copy(m_data, str, m_len * sizeof(T));
            m_data[m_len] = 0;
        }

        TString(int sz)
        {
            m_alloc   = sz + 1;
            m_len     = 0;
            m_data    = new T[m_alloc];
            m_data[0] = 0;
        }

        TString(T chr, int cnt)
        {
            m_alloc = cnt + 1;
            m_len = cnt;
            m_data = new T[m_alloc];

            for(int i = 0; i < m_len; i++)
                m_data[i] = chr;

            m_data[m_len] = 0;
        }

        ~TString()
        {
            if(m_data != nullptr)
                delete[] m_data;
        }

        void clear()
        {
            if(m_len != 0) {
                delete[] m_data;
                m_data = new T[1];
                m_data[0] = 0;

                m_alloc = 1;
                m_len = 0;
            }
        }

        void cleanup()
        {
            m_len = 0;
            m_data[0] = 0;
        }

        int length() const
        {
            return m_len;
        }

        bool isEmpty() const
        {
            return m_len <= 0;
        }

        const T *raw() const
        {
            return m_data;
        }

        T *rawCopy() const
        {
            T *ret = new T[m_len + 1];
            mem::copy(ret, m_data, m_len + 1);

            return ret;
        }

        Iterator begin()
        {
            return m_data;
        }

        Iterator end()
        {
            return m_data + m_len;
        }

        CIterator begin() const
        {
            return m_data;
        }

        CIterator end() const
        {
            return m_data + m_len;
        }

        TString<T> &reserve(int sz)
        {
            sz++; //Don't forget the \0 !
            if(m_alloc < sz)
                grow(sz);

            return *this;
        }

        TString<T> &operator += (const TString<T> &src)
        {
            reserve(m_len + src.m_len);
            mem::copy(m_data + m_len, src.m_data, (src.m_len + 1) * sizeof(T));
            m_len += src.m_len;
            return *this;
        }

        TString<T> &operator += (const T *str)
        {
            mDebugAssert(str != nullptr, "passed nullptr to string function");
            int len = 0; //Can't use strlen() for this since it can also be a wchar_t!
            while(str[len] != 0)
                len++;

            reserve(m_len + len);
            mem::copy(m_data + m_len, str, (len + 1) * sizeof(T));
            m_len += len;
            return *this;
        }

        TString<T> &operator += (T c)
        {
            reserve(m_len + 1);
            m_data[m_len] = c;
            m_data[++m_len] = 0;
            return *this;
        }

        TString<T> &operator = (const TString<T> &src)
        {
            if(m_alloc < src.m_len + 1) {
                delete[] m_data;
                m_alloc = src.m_len + 1;
                m_data = new T[m_alloc];
            }

            m_len = src.m_len;
            mem::copy(m_data, src.m_data, (m_len + 1) * sizeof(T));
            return *this;
        }

        TString<T> &operator = (TString<T> &&src)
        {
            if(m_data != nullptr)
                delete[] m_data;

            m_alloc    = src.m_alloc;
            m_len      = src.m_len;
            m_data     = src.m_data;
            src.m_data = nullptr;
            return *this;
        }
        
        TString<T> &operator = (const T *str)
        {
            mDebugAssert(str != nullptr, "passed nullptr to string function");
            int len = 0; //Can't use strlen() for this since it can also be a wchar_t!
            while(str[len] != 0)
                len++;
        
            if(m_alloc != len) {
                delete[] m_data;
                m_alloc = len + 1;
                m_data = new T[m_alloc];
            }

            m_len = len;
            mem::copy(m_data, str, (len + 1) * sizeof(T));
            return *this;
        }

        TString<T> operator + (const TString<T> &src) const
        {
            TString<T> ret(m_len + src.m_len);
            mem::copy(ret.m_data, m_data, m_len * sizeof(T));
            mem::copy(ret.m_data + m_len, src.m_data, (src.m_len + 1) * sizeof(T));

            ret.m_len = m_len + src.m_len;
            return ret;
        }

        TString<T> operator + (const T *str) const
        {
            mDebugAssert(str != nullptr, "passed nullptr to string function");
            int len = 0; //Can't use strlen() for this since it can also be a wchar_t!
            while(str[len] != 0)
                len++;

            TString<T> ret(m_len + len);
            mem::copy(ret.m_data, m_data, m_len * sizeof(T));
            mem::copy(ret.m_data + m_len, str, (len + 1) * sizeof(T));

            ret.m_len = m_len + len;
            return ret;
        }

        TString<T> operator + (T c) const
        {
            TString<T> ret(m_len + 1);
            mem::copy(ret.m_data, m_data, m_len * sizeof(T));
            ret.m_data[m_len] = c;
            ret.m_data[m_len + 1] = 0;

            ret.m_len = m_len + 1;
            return ret;
        }

        bool operator == (const TString<T> &src) const
        {
            return m_len == src.m_len && mem::cmp(m_data, src.m_data, m_len) == 0;
        }

        bool operator != (const TString<T> &src) const
        {
            return m_len != src.m_len || mem::cmp(m_data, src.m_data, m_len) != 0;
        }

        bool operator == (const T *src) const
        {
            mDebugAssert(src != nullptr, "passed nullptr to string function");
            for(int i = 0; i <= m_len; i++) {
                if(m_data[i] != src[i])
                    return false;
            }

            return true;
        }

        bool operator != (const T *src) const
        {
            mDebugAssert(src != nullptr, "passed nullptr to string function");
            for(int i = 0; i <= m_len; i++) {
                if(m_data[i] != src[i])
                    return true;
            }

            return false;
        }

        bool equals(const T *src, int len) const
        {
            mDebugAssert(src != nullptr, "passed nullptr to string function");
            if(m_len != len)
                return false;

            return mem::cmp(m_data, src, m_len) == 0;
        }

        bool equalsIgnoreCase(const TString<T> &src) const
        {
            if(m_len != src.m_len)
                return false;

            for(int i = 0; i < m_len; i++) {
                if(tolower(static_cast<char>(m_data[i])) != tolower(static_cast<char>(src.m_data[i])))
                    return false;
            }

            return true;
        }

        bool equalsIgnoreCase(const T *str) const
        {
            int i;
            for(i = 0; i < m_len; i++) {
                if(tolower(static_cast<char>(m_data[i])) != tolower(static_cast<char>(str[i])))
                    return false;
            }

            return str[i] == 0;
        }

        T &operator[] (int idx)
        {
            return m_data[idx];
        }

        T operator[] (int idx) const
        {
            return m_data[idx];
        }

        TString<T> &append(const T *str, int len = -1)
        {
            if(len < 0) {
                len = 0; //Can't use strlen() for this since it can also be a wchar_t!
                while(str[len] != 0)
                    len++;
            }

            if(len == 0)
                return *this;

            reserve(m_len + len);
            mem::copy(m_data + m_len, str, len * sizeof(T));
            m_len += len;
            m_data[m_len] = 0;
            return *this;
        }

        TString<T> &append(T chr, int cnt)
        {
            if(cnt <= 0)
                return *this;

            reserve(m_len + cnt);
            for(int i = 0; i < cnt; i++)
                m_data[m_len + i] = chr;

            m_len += cnt;
            m_data[m_len] = 0;
            return *this;
        }

        int hash() const
        {
            int ret = 0;
            int mul = 1;

            for(int i = m_len - 1; i >= 0; i--) {
                ret += static_cast<int>(m_data[i]) * mul;
                mul *= 31;
            }

            return ret;
        }

        int lowerHash() const
        {
            int ret = 0;
            int mul = 1;

            for(int i = m_len - 1; i >= 0; i--) {
                ret += static_cast<int>(tolower(static_cast<char>(m_data[i]))) * mul;
                mul *= 31;
            }

            return ret;
        }

        TString<T> &transform(std::function<T(T)> func)
        {
            for(int i = 0; i < m_len; i++)
                m_data[i] = func(m_data[i]);

            return *this;
        }

        TString<T> transformed(std::function<T(T)> func) const
        {
            TString<T> ret(m_len);
            for(int i = 0; i < m_len; i++)
                ret.m_data[i] = func(m_data[i]);

            ret.m_data[m_len] = 0;
            ret.m_len = m_len;
            return ret;
        }

        TString<T> substr(int beg, int end = -1) const
        {
            if(end < 0)
                end = m_len + end + 1;
            else if(end > m_len)
                end = m_len;

            if(beg < 0)
                beg = 0;

            int len = end - beg;
            if(len <= 0)
                return TString<T>();

            TString<T> ret(len);
            mem::copy(ret.m_data, m_data + beg, len);
            ret.m_data[len] = 0;
            ret.m_len = len;

            return ret;
        }

        int indexOf(T chr, int beg = 0) const
        {
            if(beg >= 0) {
                for(int i = beg; i < m_len; i++) {
                    if(m_data[i] == chr)
                        return i;
                }
            } else { //Reverse search
                for(int i = m_len + beg; i >= 0; i--) {
                    if(m_data[i] == chr)
                        return i;
                }
            }

            return -1; //Means not found
        }

        int indexOfAnyOf(const T *str, int beg = 0, int len = -1) const
        {
            if(len < 0) {
                len = 0;

                while(str[len] != 0)
                    len++;
            }

            if(len == 0)
                return -1;

            if(beg >= 0) {
                for(int i = beg; i < m_len; i++) {
                    for(int j = 0; j < len; j++) {
                        if(m_data[i] == str[j])
                            return i;
                    }
                }
            } else { //Reverse search
                for(int i = m_len + beg; i >= 0; i--) {
                    for(int j = 0; j < len; j++) {
                        if(m_data[i] == str[j])
                            return i;
                    }
                }
            }

            return -1; //Means not found
        }

        int lastIndexOf(T chr) const
        {
            return indexOf(chr, -1);
        }

        int indexOf(const T *str, int beg = 0, int len = -1) const
        {
            if(len < 0) {
                len = 0;

                while(str[len] != 0)
                    len++;
            }

            if(len == 0)
                return -1;

            if(beg >= 0) {
                int match = 0;

                for(int i = beg; i < m_len; i++) {
                    if(m_data[i] == str[match]) {
                        if(++match >= len) //Bingo!
                            return i - len + 1;
                    } else if(match > 0)
                        match = 0; //Doesn't match, reset
                }
            } else {
                len--; //len becomes end
                int match = len;

                for(int i = m_len + beg; i >= 0; i--) {
                    if(m_data[i] == str[match]) {
                        if(--match < 0) //Bingo!
                            return i;
                    } else if(match < len)
                        match = len; //Doesn't match, reset
                }
            }

            return -1; //Means not found
        }

        int lastIndexOf(const T *str, int len = -1) const
        {
            return indexOf(str, -1, len);
        }

        //Mix of substr() and erase()
        TString<T> take(int beg, int end = -1)
        {
            if(end < 0)
                end = m_len + end + 1;
            else if(end > m_len)
                end = m_len;

            if(beg < 0)
                beg = 0;

            int len = end - beg;
            if(len <= 0)
                return TString<T>();

            //Extract data
            TString<T> ret(len);
            mem::copy(ret.m_data, m_data + beg, len);
            ret.m_data[len] = 0;
            ret.m_len = len;

            //Move
            int remaining = m_len - end;
            if(remaining > 0)
                mem::move(m_data + beg, m_data + end, remaining);

            m_len -= len;
            m_data[m_len] = 0;
            return ret;
        }

        TString<T> &erase(int beg, int end = -1)
        {
            if(end < 0)
                end = m_len + end + 1;
            else if(end > m_len)
                end = m_len;

            if(beg < 0)
                beg = 0;

            if(end <= beg)
                return *this;

            //Move
            int remaining = m_len - end;
            if(remaining > 0)
                mem::move(m_data + beg, m_data + end, remaining);

            m_len = beg + remaining;
            m_data[m_len] = 0;
            return *this;
        }

        TString<T> &insert(int pos, const TString<T> &str)
        {
            if(pos < 0 || pos > m_len || str.m_len <= 0)
                return *this;

            reserve(m_len + str.m_len);
            mem::move(m_data + pos + str.m_len, m_data + pos, str.m_len + 1); //Copy 0 byte
            mem::copy(m_data + pos, str.m_data, str.m_len);
            return *this;
        }

        TString<T> &insert(int pos, const T *str, int len = -1)
        {
            if(pos < 0 || pos > m_len)
                return *this;

            if(len < 0) {
                len = 0;

                while(str[len] != 0)
                    len++;
            }

            if(len <= 0)
                return *this;

            reserve(m_len + len);
            mem::move(m_data + pos + len, m_data + pos, len + 1); //Copy 0 byte
            mem::copy(m_data + pos, str, len);
            return *this;
        }

        TString<T> &toLower()
        {
            for(int i = 0; i < m_len; i++)
                m_data[i] = static_cast<T>(tolower(static_cast<char>(m_data[i])));

            return *this;
        }

        TString<T> lower() const
        {
            TString<T> ret(m_len);
            for(int i = 0; i < m_len; i++)
                ret.m_data[i] = static_cast<T>(tolower(static_cast<char>(m_data[i])));

            ret.m_data[m_len] = 0;
            ret.m_len = m_len;
            return ret;
        }

        TString<T> &toUpper()
        {
            for(int i = 0; i < m_len; i++)
                m_data[i] = static_cast<T>(toupper(static_cast<char>(m_data[i])));

            return *this;
        }

        TString<T> upper() const
        {
            TString<T> ret(m_len);
            for(int i = 0; i < m_len; i++)
                ret.m_data[i] = static_cast<T>(toupper(static_cast<char>(m_data[i])));

            ret.m_data[m_len] = 0;
            ret.m_len = m_len;
            return ret;
        }

        bool isInteger(int base = 10) const
        {
            int i = (m_data[0] == '-' || m_data[0] == '+') ? 1 : 0;

            if(base > 10) {
                T max = static_cast<T>(base - 10);
                T lMax = 'a' + max;
                T uMax = 'A' + max;

                for(; i < m_len; i++) {
                    if((m_data[i] < '0' || m_data[i] > '9') && (m_data[i] < 'a' || m_data[i] > lMax) && (m_data[i] < 'A' && m_data[i] > uMax))
                        return false;
                }
            } else {
                T max = '0' + static_cast<T>(base);

                for(; i < m_len; i++) {
                    if(m_data[i] < '0' || m_data[i] > max)
                        return false;
                }
            }

            return true;
        }

        bool isNumber() const
        {
            int i = (m_data[0] == '-' || m_data[0] == '+') ? 1 : 0;
            while(i < m_len && m_data[i] >= '0' && m_data[i] <= '9')
                i++;

            if(i >= m_len)
                return true;

            if(m_data[i] == '.') {
                i++;
                while(i < m_len && m_data[i] >= '0' && m_data[i] <= '9')
                    i++;

                if(i >= m_len)
                    return true;
            }

            if(m_data[i] == 'e' || m_data[i] == 'E') {
                i++;
                if(m_data[i] == '-' || m_data[i] == '+')
                    i++;

                while(i < m_len && m_data[i] >= '0' && m_data[i] <= '9')
                    i++;

                if(i >= m_len)
                    return true;
            }

            return false;
        }

        uint32_t toUInteger(int base = 10) const
        {
            uint32_t ret = 0;

            for(int i = 0; i < m_len; i++) {
                uint32_t num;
                if(m_data[i] >= 'A' && m_data[i] <= 'Z')
                    num = static_cast<uint32_t>(m_data[i] - 'A') + 10;
                else if(m_data[i] >= 'a' && m_data[i] <= 'z')
                    num = static_cast<uint32_t>(m_data[i] - 'a') + 10;
                else if(m_data[i] >= '0' && m_data[i] <= '9')
                    num = static_cast<uint32_t>(m_data[i] - '0');
                else
                    return ret;

                ret = ret * base + num;
            }

            return ret;
        }

        int toInteger(int base = 10) const
        {
            bool hasSign = (m_data[0] == '-' || m_data[0] == '+');
            int ret = 0;

            for(int i = hasSign ? 1 : 0; i < m_len; i++) {
                int num;
                if(m_data[i] >= 'A' && m_data[i] <= 'Z')
                    num = static_cast<int>(m_data[i] - 'A') + 10;
                else if(m_data[i] >= 'a' && m_data[i] <= 'z')
                    num = static_cast<int>(m_data[i] - 'a') + 10;
                else if(m_data[i] >= '0' && m_data[i] <= '9')
                    num = static_cast<int>(m_data[i] - '0');
                else
                    return (hasSign && m_data[0] == '-') ? -ret : ret;

                ret = ret * base + num;
            }

            return (hasSign && m_data[0] == '-') ? -ret : ret;
        }

        double toDouble() const
        {
            bool hasSign = (m_data[0] == '-' || m_data[0] == '+');
            int intPart = 0;
            double fracMult = 1.0, dexp = 1.0;
            int i;
            char chr;

            //Parse integral part
            for(i = hasSign ? 1 : 0; m_data[i] != '.' && m_data[i] != 'e' && m_data[i] != 'E' && i < m_len; i++) {
                chr = m_data[i];
                if(chr < '0' || chr > '9')
                    return static_cast<double>((hasSign && m_data[0] == '-') ? -intPart : intPart);

                intPart = intPart * 10 + static_cast<int>(chr - '0');
            }

            double val = static_cast<double>(intPart);
            if(m_data[i] == '.') {
                //Parse fractional part
                i++;

                for(; m_data[i] != 'e' && m_data[i] != 'E' && i < m_len; i++) {
                    chr = m_data[i];
                    if(chr < '0' || chr > '9')
                        return (hasSign && m_data[0] == '-') ? -val : val;

                    fracMult *= 0.1;
                    val += static_cast<double>(chr - '0') * fracMult;
                }
            }

            if(m_data[i] == 'e' || m_data[i] == 'E') {
                //Parse exponent
                i++;
                bool expNeg = (m_data[i] == '-');
                int expVal = 0;

                if(expNeg || m_data[i] == '+')
                    i++;

                for(; i < m_len; i++) {
                    chr = m_data[i];
                    if(chr < '0' || chr > '9')
                        break;

                    expVal = expVal * 10 + static_cast<int>(chr - '0');
                }

                dexp = pow(10.0, static_cast<double>(expNeg ? -expVal : expVal));
            }

            if(hasSign && m_data[0] == '-') //Negative
                return -val * dexp;
            else
                return val * dexp;
        }

        TString<T> trimmedLeft() const
        {
            int i;
            for(i = 0; i < m_len; i++) {
                if(!isBlankChar(m_data[i]))
                    break;
            }

            return substr(i);
        }

        TString<T> trimmedRight() const
        {
            int i;
            for(i = m_len - 1; i >= 0; i--) {
                if(!isBlankChar(m_data[i]))
                    break;
            }

            return substr(0, i + 1);
        }

        TString<T> trimmed() const
        {
            int i;
            for(i = 0; i < m_len; i++) {
                if(!isBlankChar(m_data[i]))
                    break;
            }

            int j;
            for(j = m_len - 1; j >= 0; j--) {
                if(!isBlankChar(m_data[j]))
                    break;
            }

            return substr(i, j + 1);
        }

        bool startsWith(const TString<T> &src) const
        {
            return src.isEmpty() || (m_len >= src.m_len && mem::cmp(m_data, src.m_data, src.m_len) == 0);
        }

        bool endsWith(const TString<T> &src) const
        {
            return src.isEmpty() || (m_len >= src.m_len && mem::cmp(m_data + (m_len - src.m_len), src.m_data, src.m_len) == 0);
        }

        bool startsWith(const T *str, int len = -1) const
        {
            if(len < 0) {
                len = 0;

                while(str[len] != 0)
                    len++;
            }

            return len <= 0 || (m_len >= len && mem::cmp(m_data, str, len) == 0);
        }

        bool endsWith(const T *str, int len = -1) const
        {
            if(len < 0) {
                len = 0;

                while(str[len] != 0)
                    len++;
            }

            return len <= 0 || (m_len >= len && mem::cmp(m_data + (m_len - len), str, len) == 0);
        }

        void splitOnOneOf(const T *chars, List<TString<T>> &dst, int chrLen = -1) const
        {
            if(chrLen < 0) {
                chrLen = 0;
                while(chars[chrLen] != 0)
                    chrLen++;
            }

            if(chrLen > 0) {
                int prev = 0;

                for(int i = 0; i < m_len; i++) {
                    for(int j = 0; j < chrLen; j++) {
                        if(m_data[i] == chars[j]) {
                            //We need to split here
                            dst.add(substr(prev, i)); //Does not include the separator
                            prev = i + 1; //Exclude the separator from next match

                            break;
                        }
                    }
                }

                //Append the last match
                dst.add(substr(prev));
            }
        }

        void splitOn(T match, List<TString<T>> &dst) const
        {
            int prev = 0;
            for(int i = 0; i < m_len; i++) {
                if(m_data[i] == match) {
                    dst.add(substr(prev, i));
                    prev = i + 1;
                }
            }

            dst.add(substr(prev));
        }

        void splitOn(const T *str, List<TString<T>> &dst, int strLen = -1) const
        {
            if(strLen < 0) {
                strLen = 0;
                while(str[strLen] != 0)
                    strLen++;
            }

            if(strLen > 0) {
                int prev = 0;
                int begin = 0;
                int matches = 0;

                for(int i = 0; i < m_len; i++) {
                    if(m_data[i] == str[matches]) {
                        if(matches == 0)
                            begin = i; //Match beginning

                        if(++matches >= strLen) {
                            //We need to split here
                            dst.add(substr(prev, begin));
                            prev = i + 1; //End

                            //Reset
                            matches = 0;
                        }
                    } else if(matches > 0)
                        matches = 0; //Almost, but does not match, so reset...
                }

                //Add last one
                dst.add(substr(prev));
            }
        }

        TString<T> &replace(T src, T dst)
        {
            for(int i = 0; i < m_len; i++) {
                if(m_data[i] == src)
                    m_data[i] = dst;
            }

            return *this;
        }

        TString<T> replaced(T src, T dst) const
        {
            TString<T> ret(m_len);
            for(int i = 0; i < m_len; i++) {
                if(m_data[i] == src)
                    ret.m_data[i] = dst;
                else
                    ret.m_data[i] = m_data[i];
            }

            ret.m_data[m_len] = 0;
            ret.m_len = m_len;
            return ret;
        }

        TString<T> &trimToLength(int sz)
        {
            if(sz < 0)
                sz = 0;

            if(sz >= m_len)
                return *this;

            m_len = sz;
            m_data[sz] = 0;
            return *this;
        }

        uint8_t byte(int i) const
        {
            return reinterpret_cast<uint8_t*>(m_data)[i];
        }

        static TString<T> fromPointer(const void *ptr)
        {
            TString<T> ret(2 + sizeof(void*) * 2);
            ret.m_data[0] = static_cast<T>('0');
            ret.m_data[1] = static_cast<T>('x');

            uint8_t *bytes = reinterpret_cast<uint8_t*>(&ptr);
            for(int i = 0; i < sizeof(void*); i++) {
                uint8_t l = (bytes[sizeof(void*) - i - 1] & 0xF0) >> 4;
                uint8_t r = bytes[sizeof(void*) - i - 1] & 0x0F;

                ret.m_data[2 + i * 2] = hexChar<T>(static_cast<char>(l));
                ret.m_data[3 + i * 2] = hexChar<T>(static_cast<char>(r));
            }

            ret.m_len = 2 + sizeof(void*) * 2;
            ret.m_data[ret.m_len] = 0;
            return ret;
        }

        static TString<T> fromUInteger(uint32_t iintg, uint8_t base = 10)
        {
            int numLen = intLen(iintg, base);
            if(numLen <= 0)
                numLen = 1;

            TString<T> ret(numLen);
            for(int i = numLen - 1; i >= 0; i--) {
                char c = static_cast<char>(iintg % base);
                ret.m_data[ret.m_len + i] = hexChar<T>(c);

                iintg /= base;
            }

            ret.m_len += numLen;
            ret.m_data[ret.m_len] = 0;
            return ret;
        }

        static TString<T> fromInteger(int iintg, uint8_t base = 10)
        {
            TString<T> ret;
            if(iintg < 0) {
                ret += '-';
                iintg = -iintg;
            }

            return ret + fromUInteger(static_cast<uint32_t>(iintg), base);
        }

        static TString<T> fromUInteger64(uint64_t iintg, uint8_t base = 10)
        {
            int numLen = intLen(iintg, base);
            if(numLen <= 0)
                numLen = 1;

            TString<T> ret(numLen);
            for(int i = numLen - 1; i >= 0; i--) {
                char c = static_cast<char>(iintg % base);
                ret.m_data[ret.m_len + i] = hexChar<T>(c);

                iintg /= base;
            }

            ret.m_len += numLen;
            ret.m_data[ret.m_len] = 0;
            return ret;
        }

        static TString<T> fromInteger64(int64_t iintg, uint8_t base = 10)
        {
            TString<T> ret;
            if(iintg < 0) {
                ret += '-';
                iintg = -iintg;
            }

            return ret + fromUInteger64(static_cast<uint32_t>(iintg), base);
        }

        static TString<T> fromDouble(double f, int maxPrec = 6)
        {
            TString<T> ret(4);
            uint64_t asULL = *reinterpret_cast<uint64_t*>(&f);
            uint16_t exponent = static_cast<uint16_t>((asULL & (2047ULL << 52)) >> 52);

            if(exponent == 0) {
                ret.append("0.0", 3);
                return ret;
            }

            if(exponent == 0x7FF && (asULL & 0x000FFFFFFFFFFFFFULL) != 0) {
                ret.append("NaN", 3);
                return ret;
            }

            if((asULL & (1ULL << 63)) != 0) {
                ret += '-';
                f = -f;
            }

            if(exponent == 0x7FF) {
                //Infinity
                ret.append("inf", 3);
                return ret;
            }

            int64_t intPart = static_cast<int64_t>(f);
            double fracPartD = f - static_cast<double>(intPart);
            fracPartD *= pow(10.0, static_cast<double>(maxPrec));
            int64_t fracPart = static_cast<int64_t>(fracPartD);

            if(intPart > 0) {
                int intSz = static_cast<int>(log10(f)) + 1; //log10 is better than intLen, especially if intPart >= 1000
                ret.reserve(ret.m_len + intSz);

                for(int i = intSz - 1; i >= 0; i--) {
                    ret.m_data[ret.m_len + i] = static_cast<T>('0' + intPart % 10);
                    intPart /= 10;
                }

                ret.m_len += intSz;
            } else {
                ret.reserve(ret.m_len + 1);
                ret.m_data[ret.m_len++] = '0';
            }

            if(fracPart > 0) {
                ret.reserve(ret.m_len + maxPrec + 1);
                ret.m_data[ret.m_len++] = '.';

                for(int i = maxPrec - 1; i >= 0; i--) {
                    ret.m_data[ret.m_len + i] = static_cast<T>('0' + fracPart % 10);
                    fracPart /= 10;
                }

                ret.m_len += maxPrec;
            } else {
                ret.reserve(ret.m_len + 2);
                ret.m_data[ret.m_len + 0] = '.';
                ret.m_data[ret.m_len + 1] = '0';
                ret.m_len += 2;
            }

            ret.m_data[ret.m_len] = 0;
            return ret;
        }

        static TString<T> vformat(std::function<TString<T>(T, VAList*)> func, const T *frmt, VAList *lst)
        {
            TString<T> ret;
            int start = 0;
            T lastChr = 0;

            for(int i = 0; frmt[i] != 0; i++) {
                if(lastChr == '%') {
                    ret.append(frmt + start, i - start - 1);
                    start = i + 1;

                    if(frmt[i] == '%') {
                        start--;
                        lastChr = 0; //Whatever but not %
                    } else {
                        ret += func(frmt[i], lst);
                        lastChr = frmt[i];
                    }
                } else
                    lastChr = frmt[i];
            }

            ret += frmt + start;
            return ret;
        }

        static TString<T> vformat(const T *frmt, VAList *lst)
        {
            //Can't use std::function for that. It's too slow!!
            //I'd rather do this dirty little copy/paste than 
            //using vformat() with a lambda.

            TString<T> ret;
            int start = 0;
            T lastChr = 0;

            for(int i = 0; frmt[i] != 0; i++) {
                if(lastChr == '%') {
                    ret.append(frmt + start, i - start - 1);
                    start = i + 1;

                    if(frmt[i] == '%') {
                        start--;
                        lastChr = 0; //Whatever but not %
                    } else {
                        switch(static_cast<char>(frmt[i])) {
                        case 'i':
                        case 'd': ret += fromInteger(va_arg(lst->list, int)); break;
                        case 'h': ret += fromInteger(va_arg(lst->list, int), 16); break;
                        case 'H': ret += fromInteger(va_arg(lst->list, int), 16).toUpper(); break;
                        case 'p': ret += fromPointer(va_arg(lst->list, void*)); break;
                        case 'P': ret += fromPointer(va_arg(lst->list, void*)).toUpper(); break;
                        case 'f': ret += fromDouble(va_arg(lst->list, double)); break;
                        case 's': ret += TString<T>(va_arg(lst->list, const T*)); break;
                        case 'c': ret += TString<T>(static_cast<T>(va_arg(lst->list, int)), 1); break;
                        default:  break;
                        }

                        lastChr = frmt[i];
                    }
                } else
                    lastChr = frmt[i];
            }

            ret += frmt + start;
            return ret;
        }

        static TString<T> format(std::function<TString<T>(T, VAList*)> func, const T *frmt, ...)
        {
            VAList lst;
            va_start(lst.list, frmt);
            TString<T> ret(vformat(func, frmt, &lst));
            va_end(lst.list);

            return ret;
        }

        static TString<T> format(const T *frmt, ...)
        {
            VAList lst;
            va_start(lst.list, frmt);
            TString<T> ret(vformat(frmt, &lst));
            va_end(lst.list);

            return ret;
        }
        
        static TString<T> uninitialized(int sz)
        {
            TString<T> ret(sz);
            ret.m_len = sz;
            
            return ret;
        }

    protected:
        T *m_data;
        int m_len;
        int m_alloc;

        void grow(int targetSz)
        {
            int add = m_alloc + (m_alloc >> 1);
            if(add < targetSz)
                m_alloc = targetSz;
            else
                m_alloc = add;

            T *ndata = new T[m_alloc];
            mem::copy(ndata, m_data, (m_len + 1) * sizeof(T));
            delete[] m_data;
            m_data = ndata;
        }
    };

    template<typename T> class TConstString
    {
    public:
        TConstString(const TString<T> &str) : m_string(str)
        {
            m_hash = str.hash();
        }

        TConstString(const T *str) : m_string(str)
        {
            m_hash = m_string.hash();
        }

        const TString<T> &value() const
        {
            return m_string;
        }

        int hash() const
        {
            return m_hash;
        }

        bool operator == (const TConstString<T> &src) const
        {
            return m_hash == src.m_hash;
        }

        bool operator == (const TString<T> &str) const
        {
            return m_hash == str.hash();
        }

        bool operator == (const T *str) const
        {
            return m_string == str;
        }

        bool operator != (const TConstString<T> &src) const
        {
            return m_hash != src.m_hash;
        }

        bool operator != (const TString<T> &str) const
        {
            return m_hash != str.hash();
        }

        bool operator != (const T *str) const
        {
            return m_string != str;
        }

        bool operator > (const TConstString<T> &src) const
        {
            return m_hash > src.m_hash;
        }

        bool operator >= (const TConstString<T> &src) const
        {
            return m_hash >= src.m_hash;
        }

        bool operator < (const TConstString<T> &src) const
        {
            return m_hash < src.m_hash;
        }

        bool operator <= (const TConstString<T> &src) const
        {
            return m_hash <= src.m_hash;
        }

    private:
        TConstString()
        {
        }

        TString<T> m_string;
        int m_hash;
    };

    typedef TString<char> String;
    typedef TString<wchar_t> WString;
    typedef TConstString<char> ConstString;
    typedef TConstString<wchar_t> WConstString;

    class StringLowerHasher
    {
    public:
        static int hash(const String &s)
        {
            return s.lowerHash();
        }
    };

}
