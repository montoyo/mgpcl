/* Copyright (C) 2019 BARBOTIN Nicolas
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
#include "Cond.h"
#include "Atomic.h"

namespace m
{
    class Scheduler;

    class SchedulerTask
    {
        friend class Scheduler;

    public:
        //When the task is canceled, its pointer becomes invalid
        void cancel()
        {
            m_cancelled.set(1);
        }

    private:
        SchedulerTask(uint32_t nr) : m_nextRun(nr), m_interval(0), m_isRegular(false) {}
        SchedulerTask(uint32_t nr, uint32_t ival) : m_nextRun(nr), m_interval(ival), m_isRegular(true) {}
        ~SchedulerTask() = default;

        uint32_t m_nextRun;
        uint32_t m_interval;
        bool m_isRegular;
        std::function<void()> m_func;
        Atomic m_cancelled;
    };

    class Scheduler
    {
    public:
        Scheduler() : m_running(false), m_threads(4, "SW-") {}
        Scheduler(int threadCount) : m_running(false), m_threads(threadCount, "SW-") {}

        ~Scheduler()
        {
            stopThreads();
        }

        void prestartThreads(); //This is done automatically, but if you want...
        SchedulerTask *schedule(uint32_t delayMs, std::function<void()> func);
        SchedulerTask *scheduleAtFixedRate(uint32_t delayMs, uint32_t intervalMs, std::function<void()> func);
        void stopThreads();

        //Thread count has to be set before threads are started (that is, before a task is scheduled or prestartThreads() is called)
        void setThreadCount(int cnt)
        {
            m_threads.setCount(cnt);
        }

        int threadCount() const
        {
            return m_threads.count();
        }

    private:
        class Worker
        {
        public:
            Worker(Scheduler *p) : m_parent(p) {}
            void run();
            static void run(void *ud);

            bool isParentRunning();

            Scheduler *m_parent;
            m::Mutex m_lock;
            m::Cond m_newTask;
            m::List<SchedulerTask*> m_tasks;
        };

        void dispatchTask(SchedulerTask *t);

        volatile bool m_running;
        Mutex m_runningLock;
        Atomic m_dispatcher;
        ThreadPool m_threads;
    };
}
