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

#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <pthread.h>
#endif

namespace m
{
    class Cond;

    class MGPCL_PREFIX Mutex
    {
        friend class Cond;

    public:
        Mutex()
        {
#ifdef MGPCL_WIN
            InitializeCriticalSection(&m_cs);
#else
            m_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
        }

        ~Mutex()
        {
#ifdef MGPCL_WIN
            DeleteCriticalSection(&m_cs);
#endif
        }

        void lock()
        {
#ifdef MGPCL_WIN
            EnterCriticalSection(&m_cs);
#else
            pthread_mutex_lock(&m_mutex);
#endif
        }

        bool tryLock()
        {
#ifdef MGPCL_WIN
            return TryEnterCriticalSection(&m_cs) != FALSE;
#else
            return pthread_mutex_trylock(&m_mutex) == 0;
#endif
        }

        void unlock()
        {
#ifdef MGPCL_WIN
            LeaveCriticalSection(&m_cs);
#else
            pthread_mutex_unlock(&m_mutex);
#endif
        }

    private:
#ifdef MGPCL_WIN
        CRITICAL_SECTION m_cs;
#else
        pthread_mutex_t m_mutex;
#endif
    };
}

