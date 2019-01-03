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

#include "mgpcl/Scheduler.h"
#include "mgpcl/Time.h"

void m::Scheduler::prestartThreads()
{
    m_runningLock.lock();

    if(!m_running) {
        for(int i = 0; i < m_threads.count(); i++)
            m_threads.setUserdata(i, new Worker(this));

        m_threads.setCallback(Worker::run);
        m_threads.start();
        m_running = true;
    }

    m_runningLock.unlock();
}

m::SchedulerTask *m::Scheduler::schedule(uint32_t delayMs, std::function<void()> func)
{
    prestartThreads();

    SchedulerTask *ret = new SchedulerTask(time::getTimeMsUInt() + delayMs);
    ret->m_func = func;

    dispatchTask(ret);
    return ret;
}

m::SchedulerTask *m::Scheduler::scheduleAtFixedRate(uint32_t delayMs, uint32_t intervalMs, std::function<void()> func)
{
    prestartThreads();

    SchedulerTask *ret = new SchedulerTask(time::getTimeMsUInt() + delayMs, intervalMs);
    ret->m_func = func;

    dispatchTask(ret);
    return ret;
}

void m::Scheduler::stopThreads()
{
    m_runningLock.lock();

    if(m_running) {
        for(int i = 0; i < m_threads.count(); i++)
            static_cast<Worker*>(m_threads.userdata(i))->m_newTask.signal();

        m_running = false;
        m_runningLock.unlock();
        m_threads.joinAll();
    } else
        m_runningLock.unlock();
}

void m::Scheduler::dispatchTask(SchedulerTask *t)
{
    Worker *w = static_cast<Worker*>(m_threads.userdata(m_dispatcher.increment() % m_threads.count()));

    w->m_lock.lock();
    w->m_tasks.add(t);
    w->m_newTask.signal();
    w->m_lock.unlock();
}

void m::Scheduler::Worker::run(void *ud)
{
    static_cast<Worker*>(ud)->run();
}

bool m::Scheduler::Worker::isParentRunning()
{
    m_parent->m_runningLock.lock();
    volatile bool running = m_parent->m_running;
    m_parent->m_runningLock.unlock();

    return running;
}

void m::Scheduler::Worker::run()
{
    m_lock.lock();

    while(isParentRunning()) {
        if(m_tasks.isEmpty()) {
            m_newTask.wait(m_lock);
            continue;
        }

        int nextTask = 0;
        for(int i = 1; i < ~m_tasks; i++) {
            if(m_tasks[i]->m_nextRun < m_tasks[nextTask]->m_nextRun)
                nextTask = i;
        }

        SchedulerTask *task = m_tasks[nextTask];
        if(task->m_cancelled.get() != 0) {
            m_tasks.remove(nextTask);
            delete task;
            continue;
        }

        uint32_t now = time::getTimeMsUInt();
        bool cont = false;

        while(task->m_nextRun > now) {
            uint32_t diff = task->m_nextRun - now;
            if(m_newTask.waitFor(m_lock, diff)) {
                cont = true; //New task has arrived, this one might not be the first one to run
                break;
            }

            if(task->m_cancelled.get() != 0) {
                m_tasks.remove(nextTask);
                delete task;
                cont = true;
                break;
            }

            now = time::getTimeMsUInt();
        }

        if(cont)
            continue;

        m_lock.unlock();
        task->m_func();
        m_lock.lock();

        if(task->m_isRegular)
            task->m_nextRun = time::getTimeMsUInt() + task->m_interval;
        else {
            m_tasks.remove(nextTask);
            delete task;
        }
    }

    m_lock.unlock();
}
