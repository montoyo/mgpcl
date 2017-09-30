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
#include "Assert.h"
#include "Mem.h"
#include <cstdint>

namespace m
{
    class Bitfield
    {
    public:
        Bitfield()
        {
            m_numBits = 0;
            m_bits = nullptr;
        }

        Bitfield(uint32_t nb)
        {
            m_numBits = nb;
            m_bits = new uint32_t[nb >> 5];
        }

        Bitfield(const Bitfield &src)
        {
            m_numBits = src.m_numBits;

            if(m_numBits == 0)
                m_bits = nullptr;
            else {
                m_bits = new uint32_t[m_numBits >> 5];
                mem::copy(m_bits, src.m_bits, m_numBits >> 5);
            }
        }

        Bitfield(Bitfield &&src)
        {
            m_numBits = src.m_numBits;
            m_bits = src.m_bits;
            src.m_bits = nullptr;
        }

        ~Bitfield()
        {
            if(m_bits != nullptr)
                delete[] m_bits;
        }

        Bitfield &operator = (const Bitfield &src)
        {
            if(m_bits == src.m_bits)
                return *this;

            if(m_bits != nullptr)
                delete[] m_bits;

            m_numBits = src.m_numBits;

            if(m_numBits == 0)
                m_bits = nullptr;
            else {
                m_bits = new uint32_t[m_numBits >> 5];
                mem::copy(m_bits, src.m_bits, m_numBits >> 5);
            }

            return *this;
        }

        Bitfield &operator = (Bitfield &&src)
        {
            if(m_bits != nullptr)
                delete[] m_bits;

            m_numBits = src.m_numBits;
            m_bits = src.m_bits;
            src.m_bits = nullptr;
            return *this;
        }

        Bitfield &setBitCount(uint32_t bc)
        {
            if(m_numBits != bc) {
                if(m_bits != nullptr)
                    delete[] m_bits;

                m_numBits = bc;
                m_bits = new uint32_t[bc >> 5];
            }

            return *this;
        }

        Bitfield &clear()
        {
            if(m_bits != nullptr)
                mem::zero(m_bits, (m_numBits >> 5) * sizeof(uint32_t));

            return *this;
        }

        Bitfield &setBit(uint32_t bit)
        {
            mDebugAssert(bit < m_numBits, "bit out of range");
            const uint32_t a = (bit & 0xFFFFFFE0) >> 5;
            const uint32_t b = 1 << (bit & 0x0000001F);

            m_bits[a] |= b;
            return *this;
        }

        Bitfield &setBit(uint32_t bit, bool val)
        {
            mDebugAssert(bit < m_numBits, "bit out of range");
            const uint32_t a = (bit & 0xFFFFFFE0) >> 5;
            const uint32_t b = 1 << (bit & 0x0000001F);

            if(val)
                m_bits[a] |= b;
            else
                m_bits[a] &= ~b;

            return *this;
        }

        Bitfield &clearBit(uint32_t bit)
        {
            mDebugAssert(bit < m_numBits, "bit out of range");
            const uint32_t a = (bit & 0xFFFFFFE0) >> 5;
            const uint32_t b = 1 << (bit & 0x0000001F);

            m_bits[a] &= ~b;
            return *this;
        }

        Bitfield &toggleBit(uint32_t bit)
        {
            mDebugAssert(bit < m_numBits, "bit out of range");
            const uint32_t a = (bit & 0xFFFFFFE0) >> 5;
            const uint32_t b = 1 << (bit & 0x0000001F);

            m_bits[a] ^= b;
            return *this;
        }

        bool bit(uint32_t bit) const
        {
            mDebugAssert(bit < m_numBits, "bit out of range");
            const uint32_t a = (bit & 0xFFFFFFE0) >> 5;
            const uint32_t b = bit & 0x0000001F;

            return static_cast<bool>((m_bits[a] & (1 << b)) >> b);
        }

        uint32_t numBits() const
        {
            return m_numBits;
        }

    private:
        uint32_t m_numBits;
        uint32_t *m_bits;
    };
}
