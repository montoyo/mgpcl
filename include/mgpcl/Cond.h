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
#include "Mutex.h"
#include <cstdint>

#ifdef MGPCL_LINUX
#include <sys/time.h>
#endif

namespace m
{
	class MGPCL_PREFIX Cond
	{
	public:
		Cond()
		{
#ifdef MGPCL_WIN
			InitializeConditionVariable(&m_cv);
#else
            m_cv = PTHREAD_COND_INITIALIZER;
#endif
		}

		void wait(Mutex &m)
		{
#ifdef MGPCL_WIN
			SleepConditionVariableCS(&m_cv, &m.m_cs, INFINITE);
#else
            pthread_cond_wait(&m_cv, &m.m_mutex);
#endif
		}

		bool waitFor(Mutex &m, uint32_t ms)
		{
#ifdef MGPCL_WIN
			return SleepConditionVariableCS(&m_cv, &m.m_cs, ms) != FALSE;
#else
            struct timeval tv;
            struct timespec ts;

            if(gettimeofday(&tv, nullptr) != 0)
                return false;

            tv.tv_usec += ms * 1000;
            ts.tv_sec   = tv.tv_sec + tv.tv_usec / 1000000;
            ts.tv_nsec  = (tv.tv_usec % 1000000) * 1000;

            return pthread_cond_timedwait(&m_cv, &m.m_mutex, &ts) == 0;
#endif
		}

		void signal()
		{
#ifdef MGPCL_WIN
			WakeConditionVariable(&m_cv);
#else
            pthread_cond_signal(&m_cv);
#endif
		}

		void signalAll()
		{
#ifdef MGPCL_WIN
			WakeAllConditionVariable(&m_cv);
#else
            pthread_cond_broadcast(&m_cv);
#endif
		}

	private:
#ifdef MGPCL_WIN
		CONDITION_VARIABLE m_cv;
#else
        pthread_cond_t m_cv;
#endif
	};
}

