/* Copyright (C) 2019 BARBOTIN Nicolas
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
#include "Random.h"

namespace m
{
    /* RFC 4122 (UUID v4) compliant */
    class UUID
    {
    public:
        UUID();
        UUID(uint64_t msb, uint64_t lsb) : m_msb(msb), m_lsb(lsb) {}
        UUID(const String &str, bool strict = false); //Will be nil if parse error
        UUID(const UUID &src) : m_msb(src.m_msb), m_lsb(src.m_lsb) {}

        template<class PRNG> UUID(PRNG &p, typename std::enable_if<IsPRNG<PRNG>::value>::type *dontCare = nullptr)
        {
            m_msb = p.next();
            m_lsb = p.next();
            postGenFix();
        }

        void set(uint64_t msb, uint64_t lsb)
        {
            m_msb = msb;
            m_lsb = lsb;
        }

        template<class PRNG> void regenerate(PRNG &p)
        {
            m_msb = p.next();
            m_lsb = p.next();
            postGenFix();
        }

        void setMSB(uint64_t msb)
        {
            m_msb = msb;
        }

        void setLSB(uint64_t lsb)
        {
            m_lsb = lsb;
        }

        void setToNil()
        {
            m_msb = 0;
            m_lsb = 0;
        }

        bool isNil() const
        {
            return m_msb == 0 && m_lsb == 0;
        }

        uint64_t msb() const
        {
            return m_msb;
        }

        uint64_t lsb() const
        {
            return m_lsb;
        }

        String toString() const;
        bool setFromString(const String &str, bool strict = false);

        int hash() const
        {
            //Should be good enough
            static_assert(sizeof(int) << 1 == sizeof(uint64_t), "unsupported int size");
            const int *me = reinterpret_cast<const int*>(this);
            return me[0] ^ me[1] ^ me[2] ^ me[3];
        }

        bool operator == (const UUID &src) const
        {
            return m_msb == src.m_msb && m_lsb == src.m_lsb;
        }

        bool operator != (const UUID &src) const
        {
            return m_msb != src.m_msb || m_lsb != src.m_lsb;
        }

    private:
        void postGenFix();

        uint64_t m_msb;
        uint64_t m_lsb;
    };
}
