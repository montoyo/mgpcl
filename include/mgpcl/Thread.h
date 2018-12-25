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
#include "String.h"
#include <functional>

#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <pthread.h>
#endif

#ifdef MGPCL_THREAD_SOURCE
#define MGPCL_THREAD_EXT
#else
#define MGPCL_THREAD_EXT extern
#endif

namespace m
{
    class ThreadPool;

    class Thread
    {
        friend class ThreadPool;
        M_NON_COPYABLE(Thread)

    public:
        Thread();
        Thread(const String &name);
        virtual ~Thread();

        Thread(Thread &&src) : m_name(std::move(src.m_name))
        {
            m_running = src.m_running;

#ifdef MGPCL_WIN
            m_handle = src.m_handle;
            src.m_handle = nullptr;
#else
            m_thread = src.m_thread;
            m_isValid = src.m_isValid;
            src.m_isValid = false;
#endif
        }

        bool join();
        bool start();
        bool setAffinityMask(uint64_t mask); //Use this after .start()!

        Thread &operator = (Thread &&src)
        {
            m_name = std::move(src.m_name);
            m_running = src.m_running;

#ifdef MGPCL_WIN
            m_handle = src.m_handle;
            src.m_handle = nullptr;
#else
            m_thread = src.m_thread;
            m_isValid = src.m_isValid;
            src.m_isValid = false;
#endif

            return *this;
        }

        const String &name() const
        {
            return m_name;
        }

        bool isRunning() const
        {
            volatile bool running = m_running;
            return running;
        }

        static String currentThreadName();

    protected:
        virtual void run() = 0;

    private:
#ifdef MGPCL_WIN
        static DWORD WINAPI threadProc(LPVOID me);
        HANDLE m_handle;
#else
        static void *threadProc(void *me);
        pthread_t m_thread;
        bool m_isValid;
#endif

        volatile bool m_running;
        String m_name;
    };

    class MGPCL_PREFIX FunctionalThread : public Thread
    {
        M_NON_COPYABLE(FunctionalThread)

    public:
        FunctionalThread(std::function<void()> func);
        FunctionalThread(std::function<void()> func, const String &name);

    protected:
        void run() override;

    private:
        FunctionalThread();
        std::function<void()> m_func;
    };

    template<class T> class ClassThread : public Thread
    {
        M_NON_COPYABLE_T(ClassThread, T)

    public:
        typedef void (T::*Func)();

        ClassThread()
        {
            m_instance = nullptr;
            m_func = nullptr;
        }

        ClassThread(const String &name) : Thread(name)
        {
            m_instance = nullptr;
            m_func = nullptr;
        }

        ClassThread(T *ptr, Func f)
        {
            m_instance = ptr;
            m_func = f;
        }

        ClassThread(T *ptr, Func f, const String &name) : Thread(name)
        {
            m_instance = ptr;
            m_func = f;
        }

        ClassThread &setFunc(T *ptr, Func f)
        {
            m_instance = ptr;
            m_func = f;
            return *this;
        }

        T *instance()
        {
            return m_instance;
        }

    protected:
        void run() override
        {
            mDebugAssert(m_instance != nullptr, "forgot to set instance");
            mDebugAssert(m_func != nullptr, "forgot to set function");
            (m_instance->*m_func)();
        }

    private:
        T *m_instance;
        Func m_func;
    };

    class CallbackThread : public Thread
    {
    public:
        typedef void(*Callback)(void*);
        CallbackThread();
        CallbackThread(const String &name);
        CallbackThread(Callback cb);
        CallbackThread(Callback cb, const String &name);
        CallbackThread(Callback cb, void *ud);
        CallbackThread(Callback cb, void *ud, const String &name);

        CallbackThread &setCallback(Callback cb);
        CallbackThread &setCallback(Callback cb, void *ud);
        CallbackThread &setUserdata(void *ud);

        Callback callback() const
        {
            return m_callback;
        }

        void *userdata() const
        {
            return m_userdata;
        }

    protected:
        void run() override;

    private:
        Callback m_callback;
        void *m_userdata;
    };

    class ThreadPool
    {
    public:
        ThreadPool();
        ThreadPool(int cnt);
        ThreadPool(int cnt, const String &name);
        ~ThreadPool();

        ThreadPool &setCount(int cnt);
        ThreadPool &setName(const String &name);
        ThreadPool &setCallback(CallbackThread::Callback cb);
        ThreadPool &setCallback(CallbackThread::Callback cb, void *ud);
        ThreadPool &setUserdata(void *ud);
        ThreadPool &setUserdata(int id, void *ud);
        ThreadPool &dispatchOnCores(uint8_t numCpus);
        ThreadPool &start();
        ThreadPool &joinAll();

        int count() const
        {
            return m_count;
        }

        const String &name() const
        {
            return m_name;
        }

        bool isValid() const
        {
            return m_threads != nullptr;
        }

        bool operator ! () const
        {
            return m_threads == nullptr;
        }

        CallbackThread &access(int idx);
        CallbackThread &operator[] (int idx);

        CallbackThread::Callback callback() const;
        void *userdata() const;
        void *userdata(int id) const;

    private:
        int m_count;
        CallbackThread *m_threads;
        String m_name;
        uint8_t m_cpus;
    };

    MGPCL_THREAD_EXT bool execAsync(std::function<void()> func);

}

