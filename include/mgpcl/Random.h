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
#include "Date.h"
#include "Time.h"
#include "Config.h"
#include <cstdint>

#define M_ONE_OVER_2_POWER_64 5.421010862427522170037264004349e-20

namespace m
{
    template<class T> struct IsPRNG
    {
        template<class U> static auto testFunc(bool) -> decltype(reinterpret_cast<U*>(0)->next());
        template<class U> static void testFunc(...);

        enum
        {
            value = std::is_same<decltype(testFunc<T>(true)), uint64_t>::value
        };
    };

    namespace prng
    {
        
        class Xoroshiro
        {
        public:
            Xoroshiro()
            {
                static_assert(sizeof(uint64_t) == sizeof(double), "This won't work!");
                double t = time::getTimeMs();

                m_s[0] = static_cast<uint64_t>(Date::unixTime()) ^ 0xdeadbeef12345678;
                m_s[1] = *reinterpret_cast<uint64_t*>(&t) ^ static_cast<uint64_t>(time::getTimeMsUInt()) + 424242424242;
            }

            void setSeed(uint64_t a, uint64_t b)
            {
                m_s[0] = a;
                m_s[1] = b;
            }

            uint64_t seedA() const
            {
                return m_s[0];
            }

            uint64_t seedB() const
            {
                return m_s[1];
            }

            uint64_t seed(int p) const
            {
                mDebugAssert(p >= 0 && p < 2, "invalid seed index; must be within [0;2[");
                return m_s[p];
            }

            uint64_t next();

        private:
            uint64_t m_s[2];
        };

        static_assert(IsPRNG<Xoroshiro>::value, "Xoroshiro not a valid PRNG");

#ifndef MGPCL_NO_SSL
        class OpenSSL
        {
        public:
            uint64_t next();
        };

        static_assert(IsPRNG<OpenSSL>::value, "OpenSSL not a valid PRNG");
#endif
    }

    template<typename PRNG = prng::Xoroshiro> class Random
    {
    public:
        Random()
        {
        }

        Random(const PRNG &src)
        {
            m_prng = src;
        }

        PRNG &prng()
        {
            return m_prng;
        }

        const PRNG &prng() const
        {
            return m_prng;
        }

        int nextInt(int upper)
        {
            return static_cast<int>(m_prng.next() & 0x000000007fffffffULL) % upper;
        }

        int nextInt(int lower, int upper)
        {
            return lower + nextInt(upper - lower);
        }

        bool nextBool()
        {
            return (m_prng.next() & (1ULL << 63)) == 0;
        }

        double nextDouble(double upper)
        {
            return static_cast<double>(m_prng.next()) * M_ONE_OVER_2_POWER_64 * upper;
        }

        double nextDouble(double lower, double upper)
        {
            return lower + nextDouble(upper - lower);
        }

        uint8_t nextByte()
        {
            return static_cast<uint8_t>(m_prng.next() & 0x00000000000000ffULL);
        }

        void nextBytes(uint8_t *dst, uint32_t sz)
        {
            while(sz > sizeof(uint64_t)) {
                *reinterpret_cast<uint64_t*>(dst) = m_prng.next();
                dst += sizeof(uint64_t);
                sz -= sizeof(uint64_t);
            }

            uint64_t remaining = m_prng.next();
            uint8_t *ptr = reinterpret_cast<uint8_t*>(&remaining);

            while(sz > 0) {
                *(dst++) = *(ptr++);
                sz--;
            }
        }

    private:
        PRNG m_prng;
    };

}

