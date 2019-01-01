/* Copyright (C) 2018 BARBOTIN Nicolas
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
#include "Thread.h"
#include "ReadWriteLock.h"
#include "HashMap.h"

namespace m
{
    template<typename T> class DefaultThreadLocalAllocator
    {
    public:
        static T *allocate()
        {
            return new T;
        }
    };

    template<typename T, class Allocator = DefaultThreadLocalAllocator<T> > class ThreadLocal
    {
    public:
        T *get()
        {
            const uint64_t key = Thread::currentThreadID();
            m_lock.lockFor(RWAction::Reading);

            if(m_map.hasKey(key)) {
                T *ret = m_map[key];
                m_lock.releaseFor(RWAction::Reading);

                return ret;
            } else {
                m_lock.releaseFor(RWAction::Reading);

                T *ret = Allocator::allocate();
                m_lock.lockFor(RWAction::Writing);
                m_map[key] = ret;
                m_lock.releaseFor(RWAction::Writing);

                return ret;
            }
        }

    private:
        m::ReadWriteLock m_lock;
        m::HashMap<uint64_t, T*> m_map;
    };
}
