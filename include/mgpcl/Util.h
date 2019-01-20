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
#include <type_traits>
#include <cstdint>
#include "Config.h"
#include "Enums.h"
#include "IOStream.h"

#define M_NON_COPYABLE(cls) private: \
                                cls(const cls &src) {} \
                                cls &operator = (const cls &src) { return *this; }

#define M_NON_COPYABLE_T(cls, ...)    private: \
                                        cls(const cls<__VA_ARGS__> &src) {} \
                                        cls<__VA_ARGS__> &operator = (const cls<__VA_ARGS__> &src) { return *this; }

#define M_MAXSZ(t1, t2) (sizeof(t1) > sizeof(t2) ? sizeof(t1) : sizeof(t2))

#ifdef M_UTIL_DECLARE
#define M_UTIL_PREFIX
#else
#define M_UTIL_PREFIX extern
#endif

#define M_CRC32_INIT 0xFFFFFFFFU

namespace m
{
    //So this is a bit of a hack
    //But the whole SFINAE thing is a hack; isn't it?
    template<class T> struct HasEqualOperator
    {
        template<class U> static auto testFunc(bool) -> decltype(*reinterpret_cast<const U*>(0) == *reinterpret_cast<const U*>(0));
        template<class U> static void testFunc(...);

        enum
        {
            value = std::is_same<decltype(testFunc<T>(true)), bool>::value
        };
    };

    template<class T> struct HasNotEqualOperator
    {
        template<class U> static auto testFunc(bool) -> decltype(*reinterpret_cast<const U*>(0) != *reinterpret_cast<const U*>(0));
        template<class U> static void testFunc(...);

        enum
        {
            value = std::is_same<decltype(testFunc<T>(true)), bool>::value
        };
    };

    template<typename T> int intLen(T value, uint8_t base = 10)
    {
        static_assert(std::is_integral<T>::value, "m::intLen() only works with integral types!");

        int ret = 0;
        while(value > 0) {
            value /= static_cast<T>(base);
            ret++;
        }

        return ret;
    }

    template<typename T> T hexChar(char idx)
    {
        return static_cast<T>(idx < 10 ? '0' + idx : 'a' + (idx - 10));
    }

    //hexVal returns 0xFF if chr is invalid (i.e. not within [0-9a-fA-F])
    static inline uint8_t hexVal(char chr)
    {
        if(chr >= '0' && chr <= '9')
            return static_cast<uint8_t>(chr - '0');
        else if(chr >= 'a' && chr <= 'f')
            return static_cast<uint8_t>(chr - 'a' + 10);
        else if(chr >= 'A' && chr <= 'F')
            return static_cast<uint8_t>(chr - 'A' + 10);

        return 0xFF;
    }

    template<typename T> class TString;
    template<typename T, typename Size> class List;

    namespace IO
    {
        M_UTIL_PREFIX bool readFully(InputStream *dst, uint8_t *data, int len);
        M_UTIL_PREFIX bool writeFully(OutputStream *dst, const uint8_t *data, int len);
        M_UTIL_PREFIX bool transfer(OutputStream *dst, InputStream *src, int bufSz = 65536); //dst can be null, equivalent to '> /dev/null'
        M_UTIL_PREFIX bool writeLine(const TString<char> &str, OutputStream *dst, LineEnding le = M_OS_LINEENDING);
    }

    M_UTIL_PREFIX bool parseArgs(const TString<char> &str, List<TString<char>, int> &dst, bool strict = true);
    M_UTIL_PREFIX void makeSizeString(uint64_t sz, TString<char> &dst);
    M_UTIL_PREFIX void hexString(const uint8_t *data, uint32_t sz, TString<char> &dst);
    M_UTIL_PREFIX uint32_t unHexString(const TString<char> &src, uint8_t *dst, uint32_t bufSz);
    M_UTIL_PREFIX void base64Encode(const uint8_t *data, uint32_t sz, TString<char> &dst);
    M_UTIL_PREFIX bool base64Decode(const char *data, uint8_t *dst, uint32_t &sz, int dataLen = -1);
    M_UTIL_PREFIX bool base64Decode(const TString<char> &str, uint8_t *dst, uint32_t &sz);

    /* CRC32 computation function
     * --------------------------
     * Polynomial: 0x4C11DB7
     * Initial value: 0xFFFFFFFF (unless otherwise specified by your call)
     * Output not XOR-ed.
     * Input and output bits are not reversed.
     */
    M_UTIL_PREFIX uint32_t crc32(const uint8_t *data, uint32_t len, uint32_t crc = M_CRC32_INIT);

}
