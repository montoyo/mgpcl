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

#include "mgpcl/UUID.h"
#include "mgpcl/Random.h"

#define M_UUID_VERSION_MASK 0xF000ULL
#define M_UUID_V4_BITS 0x4000ULL
#define M_UUID_SEQ_MASK 0xC000000000000000ULL
#define M_UUID_SEQ_BITS 0x8000000000000000ULL

m::UUID::UUID()
{
    prng::Xoroshiro x;
    regenerate(x);
}

static void hex2str(uint64_t num, int numChars, m::String &dst)
{
    while(numChars > 0) {
        numChars--;

        char chr = static_cast<char>((num >> (numChars << 2)) & 15);
        dst += m::hexChar<char>(chr);
    }
}

m::String m::UUID::toString() const
{
    m::String ret(36);
    hex2str((m_msb & 0xFFFFFFFF00000000ULL) >> 32, 8, ret);
    ret += '-';
    hex2str((m_msb & 0x00000000FFFF0000ULL) >> 16, 4, ret);
    ret += '-';
    hex2str(m_msb & 0x000000000000FFFFULL, 4, ret);
    ret += '-';
    hex2str((m_lsb & 0xFFFF000000000000ULL) >> 48, 4, ret);
    ret += '-';
    hex2str(m_lsb & 0x0000FFFFFFFFFFFFULL, 12, ret);

    return ret;
}

void m::UUID::postGenFix()
{
    m_msb = (m_msb & ~M_UUID_VERSION_MASK) | M_UUID_V4_BITS;
    m_lsb = (m_lsb & ~M_UUID_SEQ_MASK) | M_UUID_SEQ_BITS;
}
