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

#define MGPCL_THREAD_SOURCE
#include "mgpcl/Thread.h"
#include "mgpcl/ReadWriteLock.h"
#include "mgpcl/HashMap.h"

#if defined(MGPCL_WIN) && defined(_DEBUG)
//From https://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

static void SetThreadName(DWORD dwThreadID, const char* threadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
    __try {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), reinterpret_cast<ULONG_PTR*>(&info));
    } __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
#pragma warning(pop)
}
#endif

//Thread procs
#ifdef MGPCL_WIN
DWORD WINAPI m::Thread::threadProc(LPVOID me)
{
    Thread *thread = static_cast<Thread*>(me);
    thread->m_running = true;
    thread->run();
    thread->m_running = false;
    return 0;
}
#else
#include <sys/prctl.h>
#include <sched.h>

void *m::Thread::threadProc(void *me)
{
    Thread *thread = static_cast<Thread*>(me);

    //FIXME: This could lead to serious crashes if name is changed right after the thread is started
    prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(thread->m_name.raw()), 0, 0, 0);

    thread->m_running = true;
    thread->run();
    thread->m_running = false;
    return nullptr;
}
#endif

//Thread name mapping for windows
#ifdef MGPCL_WIN
static m::ReadWriteLock g_mappingLock;
static m::HashMap<DWORD, m::String> g_mapping;

static class ThreadInit
{
public:
    ThreadInit()
    {
        g_mapping[GetCurrentThreadId()] = m::String("MAIN");
    }
} g_threadInit;
#endif

//Main thread reference for linux
#ifdef MGPCL_LINUX
static pthread_t g_mainThread = pthread_self();
#endif

m::Thread::Thread()
{
    m_running = false;

#ifdef MGPCL_WIN
    m_handle = nullptr;
#else
    m_isValid = false;
#endif
}

m::Thread::Thread(const String &name) : m_name(name)
{
    m_running = false;

#ifdef MGPCL_WIN
    m_handle = nullptr;
#else
    m_isValid = false;

    if(m_name.length() >= 15)
        m_name = m_name.substr(0, 15); //Pthread limitsssss....
#endif
}

m::Thread::~Thread()
{
#ifdef MGPCL_WIN
    if(m_handle != nullptr)
        CloseHandle(m_handle);
#else
#endif
}

bool m::Thread::start()
{
#ifdef MGPCL_WIN
    DWORD tid;
    m_handle = CreateThread(nullptr, 0, threadProc, this, CREATE_SUSPENDED, &tid);
    if(m_handle == nullptr)
        return false;

    g_mappingLock.lockFor(RWAction::Writing);
    g_mapping[tid] = m_name;
    g_mappingLock.releaseFor(RWAction::Writing);

#ifdef _DEBUG
    if(!m_name.isEmpty())
        SetThreadName(tid, m_name.raw());
#endif

    if(ResumeThread(m_handle) == static_cast<DWORD>(-1))
        return false;
#else
    if(pthread_create(&m_thread, nullptr, threadProc, this) != 0)
        return false;

    m_isValid = true;
#endif

    return true;
}

bool m::Thread::join()
{
#ifdef MGPCL_WIN
    if(m_handle == nullptr)
        return false;

    DWORD ret;
    do {
        ret = WaitForSingleObject(m_handle, INFINITE);
    } while(ret == WAIT_TIMEOUT);

    return ret == WAIT_OBJECT_0;
#else
    if(!m_isValid)
        return false;

    return pthread_join(m_thread, nullptr) == 0;
#endif
}

bool m::Thread::setAffinityMask(uint64_t mask)
{
#ifdef MGPCL_WIN
    if(m_handle == nullptr)
        return false;

    return SetThreadAffinityMask(m_handle, mask) != 0;
#else
    if(!m_isValid)
        return false;

    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);

    for(uint64_t i = 0; i < 64ULL; i++) {
        if(mask & (1ULL << i))
            CPU_SET(i, &cpuSet);
    }

    return pthread_setaffinity_np(m_thread, sizeof(cpu_set_t), &cpuSet) == 0;
#endif
}


m::FunctionalThread::FunctionalThread()
{
    //Private constructor
}

m::FunctionalThread::FunctionalThread(std::function<void()> func)
{
    m_func = func;
}

m::FunctionalThread::FunctionalThread(std::function<void()> func, const String &name) : Thread(name)
{
    m_func = func;
}

void m::FunctionalThread::run()
{
    m_func();
}

m::String m::Thread::currentThreadName()
{
#ifdef MGPCL_WIN
    String ret;
    DWORD cur = GetCurrentThreadId();

    g_mappingLock.lockFor(RWAction::Reading);
    if(g_mapping.hasKey(cur))
        ret = g_mapping[cur];
    
    g_mappingLock.releaseFor(RWAction::Reading);
    return ret;
#else
    pthread_t self = pthread_self();
    if(pthread_equal(self, g_mainThread) != 0)
        return String("MAIN");

    char tmp[16];
    return pthread_getname_np(self, tmp, 16) == 0 ? String(tmp) : String();
#endif
}


m::CallbackThread::CallbackThread()
{
    m_callback = nullptr;
    m_userdata = nullptr;
}

m::CallbackThread::CallbackThread(const String &name) : Thread(name)
{
    m_callback = nullptr;
    m_userdata = nullptr;
}

m::CallbackThread::CallbackThread(Callback cb)
{
    m_callback = cb;
    m_userdata = nullptr;
}

m::CallbackThread::CallbackThread(Callback cb, const String &name) : Thread(name)
{
    m_callback = cb;
    m_userdata = nullptr;
}

m::CallbackThread::CallbackThread(Callback cb, void *ud)
{
    m_callback = cb;
    m_userdata = ud;
}

m::CallbackThread::CallbackThread(Callback cb, void *ud, const String &name) : Thread(name)
{
    m_callback = cb;
    m_userdata = ud;
}

m::CallbackThread &m::CallbackThread::setCallback(Callback cb)
{
    m_callback = cb;
    return *this;
}

m::CallbackThread &m::CallbackThread::setCallback(Callback cb, void *ud)
{
    m_callback = cb;
    m_userdata = ud;
    return *this;
}

m::CallbackThread &m::CallbackThread::setUserdata(void *ud)
{
    m_userdata = ud;
    return *this;
}

void m::CallbackThread::run()
{
    mDebugAssert(m_callback != nullptr, "forgot to set callback");
    m_callback(m_userdata);
}


m::ThreadArray::ThreadArray()
{
    m_count = 0;
    m_threads = nullptr;
    m_cpus = 0;
}

m::ThreadArray::ThreadArray(int cnt)
{
    mDebugAssert(cnt > 0, "can't have a negative amount of threads");

    m_count = cnt;
    m_threads = (cnt == 0) ? nullptr : new CallbackThread[cnt];
    m_cpus = 0;
}

m::ThreadArray::ThreadArray(int cnt, const String &name) : m_name(name)
{
    mDebugAssert(cnt > 0, "can't have a negative amount of threads");

    m_count = cnt;
    m_threads = (cnt == 0) ? nullptr : new CallbackThread[cnt];
    m_cpus = 0;
}

m::ThreadArray::~ThreadArray()
{
    if(m_threads != nullptr)
        delete[] m_threads;
}

m::ThreadArray &m::ThreadArray::setCount(int cnt)
{
    mDebugAssert(cnt > 0, "can't have a negative amount of threads");
    if(m_threads != nullptr)
        delete[] m_threads;

    m_count = cnt;
    if(cnt == 0)
        m_threads = nullptr;
    else
        m_threads = new CallbackThread[cnt];

    return *this;
}

m::ThreadArray &m::ThreadArray::setName(const String &name)
{
    m_name = name;
    return *this;
}

m::ThreadArray &m::ThreadArray::setCallback(CallbackThread::Callback cb)
{
    mDebugAssert(m_threads != nullptr, "forgot to set thread count");
    for(int i = 0; i < m_count; i++)
        m_threads[i].setCallback(cb);

    return *this;
}

m::ThreadArray &m::ThreadArray::setCallback(CallbackThread::Callback cb, void *ud)
{
    mDebugAssert(m_threads != nullptr, "forgot to set thread count");
    for(int i = 0; i < m_count; i++)
        m_threads[i].setCallback(cb, ud);

    return *this;
}

m::ThreadArray &m::ThreadArray::setUserdata(void *ud)
{
    mDebugAssert(m_threads != nullptr, "forgot to set thread count");
    for(int i = 0; i < m_count; i++)
        m_threads[i].setUserdata(ud);

    return *this;
}

m::ThreadArray &m::ThreadArray::setUserdata(int id, void *ud)
{
    mDebugAssert(m_threads != nullptr, "forgot to set thread count");
    mDebugAssert(id >= 0 && id < m_count, "invalid thread index");

    m_threads[id].setUserdata(ud);
    return *this;
}

m::ThreadArray &m::ThreadArray::dispatchOnCores(uint8_t numCpus)
{
    m_cpus = numCpus;
    return *this;
}

m::ThreadArray &m::ThreadArray::start()
{
    mDebugAssert(m_threads != nullptr, "forgot to set thread count");

    for(int i = 0; i < m_count; i++) {
        m_threads[i].m_name = m_name + String::fromUInteger(static_cast<uint32_t>(i));
        m_threads[i].start();

        if(m_cpus != 0) {
            int core = i % static_cast<int>(m_cpus);
            uint64_t mask = 1ULL << static_cast<uint64_t>(core);

            m_threads[i].setAffinityMask(mask);
        }
    }

    return *this;
}

m::ThreadArray &m::ThreadArray::joinAll()
{
    mDebugAssert(m_threads != nullptr, "forgot to set thread count");
    for(int i = 0; i < m_count; i++)
        m_threads[i].join();

    return *this;
}

m::CallbackThread::Callback m::ThreadArray::callback() const
{
    mDebugAssert(m_threads != nullptr, "forgot to set thread count");
    return m_threads->callback();
}

void *m::ThreadArray::userdata() const
{
    mDebugAssert(m_threads != nullptr, "forgot to set thread count");
    return m_threads->userdata();
}

void *m::ThreadArray::userdata(int id) const
{
    mDebugAssert(m_threads != nullptr, "forgot to set thread count");
    mDebugAssert(id >= 0 && id < m_count, "invalid thread index");
    return m_threads[id].userdata();
}

m::CallbackThread &m::ThreadArray::access(int idx)
{
    mDebugAssert(m_threads != nullptr, "forgot to set thread count");
    mDebugAssert(idx >= 0 && idx < m_count, "invalid thread index");

    return m_threads[idx];
}

m::CallbackThread &m::ThreadArray::operator[] (int idx)
{
    mDebugAssert(m_threads != nullptr, "forgot to set thread count");
    mDebugAssert(idx >= 0 && idx < m_count, "invalid thread index");

    return m_threads[idx];
}

#ifdef MGPCL_WIN

static DWORD WINAPI g_stdFuncThreadProc(LPVOID func)
{
    auto realFunc = static_cast<std::function<void()>*>(func);
    std::function<void()> funcCpy(*realFunc);
    delete realFunc;
    funcCpy();
    return 0;
}

bool m::execAsync(std::function<void()> func)
{
    auto ptrFunc = new std::function<void()>(func);
    HANDLE handle = CreateThread(nullptr, 0, g_stdFuncThreadProc, ptrFunc, 0, nullptr);

    if(handle == nullptr) {
        delete ptrFunc;
        return false;
    }

    CloseHandle(handle);
    return true;
}

#else

void *g_stdFuncThreadProc(void *func)
{
    auto realFunc = static_cast<std::function<void()>*>(func);
    std::function<void()> funcCpy(*realFunc);
    delete realFunc;
    funcCpy();
    return 0;
}

bool m::execAsync(std::function<void()> func)
{
    auto ptrFunc = new std::function<void()>(func);
    pthread_attr_t attrs;
    pthread_t thread;

    pthread_attr_init(&attrs);
    pthread_attr_setdetachstate(&attrs, 1);

    if(pthread_create(&thread, &attrs, g_stdFuncThreadProc, ptrFunc) == 0)
        return true;

    delete ptrFunc;
    return false;
}

#endif
