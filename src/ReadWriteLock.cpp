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

#include "mgpcl/ReadWriteLock.h"

m::ReadWriteLock::ReadWriteLock()
{
    m_reads = 0;
    m_writing = false;
}

m::ReadWriteLock::~ReadWriteLock()
{
}

void m::ReadWriteLock::lockFor(RWAction t)
{
    switch(t) {
    case RWAction::Reading:
        m_lock.lock();
        while(m_writing)
            m_write.wait(m_lock);

        m_reads++;
        m_lock.unlock();
        break;

    case RWAction::Writing:
        m_lock.lock();
        while(m_reads > 0)
            m_read.wait(m_lock);

        while(m_writing)
            m_write.wait(m_lock);

        m_writing = true;
        m_lock.unlock();
        break;
    }
}

void m::ReadWriteLock::releaseFor(RWAction t)
{
    switch(t) {
    case RWAction::Reading:
        m_lock.lock();
        if(--m_reads == 0)
            m_read.signalAll();

        m_lock.unlock();
        break;

    case RWAction::Writing:
        m_lock.lock();
        m_writing = false;
        m_write.signalAll();
        m_lock.unlock();
        break;
    }
}

bool m::ReadWriteLock::tryLockForWriting()
{
    bool ret = true;

    m_lock.lock();
    if(m_reads > 0 || m_writing)
        ret = false;
    else
        m_writing = true;

    m_lock.unlock();
    return ret;
}
