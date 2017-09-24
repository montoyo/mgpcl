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

#if defined(MGPCL_WIN)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace m
{
    class MGPCL_PREFIX Atomic
    {
    public:
        Atomic()
        {
            m_data = 0;
        }

        Atomic(long val)
        {
            m_data = val;
        }

        /*
         * Increments the atomic number.
         * Returns true if it's now equals to zero.
         *
         */
        bool increment()
        {
            return InterlockedIncrement(&m_data) == 0;
        }

        /*
         * Decrements the atomic number.
         * Returns true if it's now equals to zero.
         *
         */
        bool decrement()
        {
            return InterlockedDecrement(&m_data) == 0;
        }

        long get()
        {
            long ret;

            do {
                ret = m_data;
            } while(InterlockedCompareExchange(&m_data, ret, ret) != ret);

            return ret;
        }

        void set(long val)
        {
            InterlockedExchange(&m_data, val);
        }

    private:
        volatile LONG m_data;
    };
}

#else

namespace m
{
    class MGPCL_PREFIX Atomic
    {
    public:
        Atomic()
        {
            m_data = 0;
        }

        Atomic(long val)
        {
            m_data = val;
        }

        /*
        * Increments the atomic number.
        * Returns true if it's now equals to zero.
        *
        */
        bool increment()
        {
            return __sync_add_and_fetch(&m_data, 1) == 0;
        }

        /*
        * Decrements the atomic number.
        * Returns true if it's now equals to zero.
        *
        */
        bool decrement()
        {
            return __sync_sub_and_fetch(&m_data, 1) == 0;
        }

        long get()
        {
            long ret;

            do {
                ret = m_data;
            } while(!__sync_bool_compare_and_swap(&m_data, ret, ret));

            return ret;
        }

        void set(long val)
        {
            /*long old;

            do {
                old = m_data;
            } while(!__sync_bool_compare_and_swap(&m_data, old, val));*/

            __sync_lock_test_and_set(&m_data, val);
        }

    private:
        volatile long m_data;
    };
}

#endif

