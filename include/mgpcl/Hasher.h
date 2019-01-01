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
#include <cstdint>

namespace m
{
    template<class T> class DefaultHasher
    {
    public:
        static int hash(const T &src)
        {
            return src.hash();
        }
    };

    template<typename T> class StaticCastHasher
    {
    public:
        static int hash(T i)
        {
            return static_cast<int>(i);
        }
    };

    template<> class DefaultHasher<long long>
    {
    public:
        static int hash(long long i_)
        {
            uint64_t i = static_cast<uint64_t>(i_);
            i = ((i >> 33) ^ i) * 0xff51afd7ed558ccd;
            i = ((i >> 33) ^ i) * 0xc4ceb9fe1a85ec53;
            i = (i >> 33) ^ i;

            return static_cast<int>(i);
        }
    };

    template<> class DefaultHasher<int>
    {
    public:
        static int hash(int i_)
        {
            uint32_t i = static_cast<uint32_t>(i_);
            i = ((i >> 16) ^ i) * 0x45d9f3b;
            i = ((i >> 16) ^ i) * 0x45d9f3b;
            i = (i >> 16) ^ i;

            return static_cast<int>(i);
        }
    };

    template<> class DefaultHasher<short> : public StaticCastHasher<short>
    {
    };

    template<> class DefaultHasher<char> : public StaticCastHasher<char>
    {
    };

    template<> class DefaultHasher<long> : public StaticCastHasher<long>
    {
    };

    template<> class DefaultHasher<unsigned long long>
    {
    public:
        static int hash(unsigned long long i)
        {
            i = ((i >> 33) ^ i) * 0xff51afd7ed558ccd;
            i = ((i >> 33) ^ i) * 0xc4ceb9fe1a85ec53;
            i = (i >> 33) ^ i;

            return static_cast<int>(i);
        }
    };

    template<> class DefaultHasher<unsigned int>
    {
    public:
        static int hash(uint32_t i)
        {
            i = ((i >> 16) ^ i) * 0x45d9f3b;
            i = ((i >> 16) ^ i) * 0x45d9f3b;
            i = (i >> 16) ^ i;

            return static_cast<int>(i);
        }
    };

    template<> class DefaultHasher<unsigned short> : public StaticCastHasher<unsigned short>
    {
    };

    template<> class DefaultHasher<unsigned char> : public StaticCastHasher<unsigned char>
    {
    };

    template<> class DefaultHasher<unsigned long> : public StaticCastHasher<unsigned long>
    {
    };

    template<> class DefaultHasher<float>
    {
    public:
        static int hash(float i)
        {
            static_assert(sizeof(float) == sizeof(int), "sizeof(float) != sizeof(int)");
            return *reinterpret_cast<int*>(&i);
        }
    };

}
