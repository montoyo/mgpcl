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

m::UUID::UUID(const String &str, bool strict) : m_msb(0), m_lsb(0)
{
    setFromString(str, strict);
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
    String ret(36);
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

template<typename T> bool parseHex(const char *str, int start, int len, T &ret)
{
    ret = 0;

    for(int i = 0; i < len; i++) {
        char chr = str[start + i];

        if(chr >= '0' && chr <= '9')
            ret = (ret << 4) | static_cast<T>(chr - '0');
        else if(chr >= 'a' && chr <= 'f')
            ret = (ret << 4) | static_cast<T>(chr - 'a' + 10);
        else if(chr >= 'A' && chr <= 'F')
            ret = (ret << 4) | static_cast<T>(chr - 'A' + 10);
        else
            return false;
    }

    return true;
}

bool m::UUID::setFromString(const String &str, bool strict)
{
    uint32_t p1;
    uint16_t p2[3];
    uint64_t p3;

    if(strict) {
        if(str.length() != 36)
            return false;

        if(str[8] != '-' || str[13] != '-' || str[18] != '-' || str[23] != '-')
            return false;

        if(!parseHex(str.raw(), 0, 8, p1) || !parseHex(str.raw(), 9, 4, p2[0]) || !parseHex(str.raw(), 14, 4, p2[1]) || !parseHex(str.raw(), 19, 4, p2[2]) || !parseHex(str.raw(), 24, 12, p3))
            return false;
    } else {
        if(str.length() < 9)
            return false; //Too short to be a valid UUID

        List<String> parts;
        str.splitOn('-', parts);

        if(~parts != 5)
            return false;

        if(parts[0].length() > 8 || !parseHex(parts[0].raw(), 0, parts[0].length(), p1))
            return false;

        for(int i = 1; i < 4; i++) {
            if(parts[i].length() > 4 || !parseHex(parts[i].raw(), 0, parts[i].length(), p2[i - 1]))
                return false;
        }

        if(parts[4].length() > 12 || !parseHex(parts[4].raw(), 0, parts[4].length(), p3))
            return false;
    }

    uint64_t msb = (static_cast<uint64_t>(p1) << 32) | (static_cast<uint64_t>(p2[0]) << 16) | static_cast<uint64_t>(p2[1]);
    uint64_t lsb = (static_cast<uint64_t>(p2[2]) << 48) | p3;

    if(msb != 0 || lsb != 0) { //Allows nil UUID to be parsed
        if((msb & M_UUID_VERSION_MASK) != M_UUID_V4_BITS)
            return false; //Not a v4 UUID

        if((lsb & M_UUID_SEQ_MASK) != M_UUID_SEQ_BITS)
            return false; //Not a v4 UUID
    }

    m_msb = msb;
    m_lsb = lsb;
    return true;
}

