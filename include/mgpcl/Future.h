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
#include "Cond.h"
#include "RefCounter.h"
#include <exception>

namespace m
{
    template<typename T> class Promise;
    template<typename T> class Future;

    template<typename T> class PromiseFutureData
    {
        template<typename U> friend class Promise;
        template<typename U> friend class Future;

    private:
        PromiseFutureData() : m_available(false), m_refs(1)
        {
        }

        void releaseRef()
        {
            if(m_refs.releaseRef())
                delete this;
        }

        Mutex m_mutex;
        Cond m_cond;
        bool m_available;
        T m_data;
        AtomicRefCounter m_refs;
    };

    template<typename T> class Future
    {
        template<typename U> friend class Promise;

    public:
        Future(const Future<T> &src) : m_data(src.m_data)
        {
            m_data->m_refs.addRef();
        }

        Future(Future<T> &&src)
        {
            m_data = src.m_data;
            src.m_data = nullptr;
        }

        ~Future()
        {
            if(m_data != nullptr)
                m_data->releaseRef();
        }

        T &get()
        {
            m_data->m_mutex.lock();
            while(!m_data->m_available)
                m_data->m_cond.wait(m_data->m_mutex);

            m_data->m_mutex.unlock();
            return m_data->m_data;
        }

        void wait()
        {
            m_data->m_mutex.lock();
            while(!m_data->m_available)
                m_data->m_cond.wait(m_data->m_mutex);

            m_data->m_mutex.unlock();
        }

        bool waitFor(uint32_t ms)
        {
            m_data->m_mutex.lock();
            if(!m_data->m_available)
                m_data->m_cond.waitFor(m_data->m_mutex, ms);

            bool ret = m_data->m_available;
            m_data->m_mutex.unlock();
            return ret;
        }

        bool isAvailable()
        {
            m_data->m_mutex.lock();
            bool ret = m_data->m_available;
            m_data->m_mutex.unlock();

            return ret;
        }

        Future<T> &operator = (const Future<T> &src)
        {
            if(m_data == src.m_data)
                return *this;

            m_data->releaseRef();
            m_data = src.m_data;
            src.m_data->m_refs.addRef();

            return *this;
        }

        Future<T> &operator = (Future<T> &&src)
        {
            m_data->releaseRef();
            m_data = src.m_data;
            src.m_data = nullptr;

            return *this;
        }

    private:
        Future() {}
        Future(PromiseFutureData<T> *pfd) : m_data(pfd)
        {
            pfd->m_refs.addRef();
        }

        PromiseFutureData<T> *m_data;
    };

    class PromiseAlreadySetException : public std::exception
    {
    public:
        PromiseAlreadySetException() throw() : std::exception("m::Promise::set(): already set")
        {
        }
    };

    template<typename T> class Promise
    {
    public:
        Promise()
        {
            m_data = new PromiseFutureData<T>();
        }

        Promise(const Promise<T> &src)
        {
            m_data = src.m_data;
            m_data->m_refs.addRef();
        }

        Promise(Promise<T> &&src)
        {
            m_data = src.m_data;
            src.m_data = nullptr;
        }

        ~Promise()
        {
            if(m_data != nullptr)
                m_data->releaseRef();
        }

        void set(const T &val)
        {
            m_data->m_mutex.lock();
            if(m_data->m_available) {
                m_data->m_mutex.unlock();
                throw PromiseAlreadySetException();
            }

            m_data->m_data = val;
            m_data->m_available = true;
            m_data->m_cond.signalAll();
            m_data->m_mutex.unlock();
        }

        void set(T &&val)
        {
            m_data->m_mutex.lock();
            if(m_data->m_available) {
                m_data->m_mutex.unlock();
                throw PromiseAlreadySetException();
            }

            m_data->m_data = val;
            m_data->m_available = true;
            m_data->m_cond.signalAll();
            m_data->m_mutex.unlock();
        }

        Future<T> makeNewFuture()
        {
            return Future<T>(m_data);
        }

        Promise<T> &operator = (const Promise<T> &src)
        {
            if(m_data == src.m_data)
                return *this;

            m_data->releaseRef();
            m_data = src.m_data;
            m_data->m_refs.addRef();

            return *this;
        }

        Promise<T> &operator = (Promise<T> &&src)
        {
            m_data->releaseRef();
            m_data = src.m_data;
            src.m_data = nullptr;

            return *this;
            return this;
        }

    private:
        PromiseFutureData<T> *m_data;
    };

}
