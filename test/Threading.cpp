#include "TestAPI.h"
#include <mgpcl/Thread.h>
#include <mgpcl/Time.h>
#include <mgpcl/Future.h>

Declare Test("threading"), Priority(9.0);

TEST
{
    static StackIntegrityChecker sic;
    testAssert(m::Thread::currentThreadName() == "MAIN"_m, "current thread name isn't main!");

    m::Mutex mtx;
    volatile bool ran = false;
    volatile bool err = false;

    m::FunctionalThread ft([&mtx, &ran, &err] ()
    {
        m::String tname(m::Thread::currentThreadName());
        std::cout << "[i]\tOther thread name is " << tname.raw() << std::endl;

        if(tname != "TEST"_m)
            err = true;

        mtx.lock();
        ran = true;
        mtx.unlock();
    }, "TEST"_m);

    double start = m::time::getTimeMs();
    testAssert(ft.start(), "could not start thread");

    while(m::time::getTimeMs() - start < 5000.0) {
        mtx.lock();
        volatile bool cpy = ran;
        mtx.unlock();

        if(cpy)
            break;
        else
            m::time::sleepMs(10);
    }

    testAssert(ran, "thread didn't run");
    testAssert(!err, "thread had wrong name");

    std::cout << "[i]\tThread ran in " << m::time::getTimeMs() - start << " ms." << std::endl;
    ft.join();
    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;
    m::Promise<int> promise;
    m::Future<int> future(promise.makeNewFuture());

    testAssert(m::execAsync([&promise] () {
        m::time::sleepMs(250);
        promise.set(42);
    }), "couldn't start async func");

    testAssert(future.get() == 42, "future != promise");
    return true;
}
