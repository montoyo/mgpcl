#define M_TESTAPI_SRC
#include "TestAPI.h"
#include <mgpcl/List.h>
#include <mgpcl/ConsoleUtils.h>
#include <sstream>

class TestData
{
public:
    TestData()
    {
        name = nullptr;
        hasPriority = false;
        priority = 0.0;
    }

    TestData(const char *n)
    {
        name = n;
        hasPriority = false;
        priority = 0.0;
    }

    TestData(double p)
    {
        name = nullptr;
        hasPriority = true;
        priority = p;
    }

    bool isValid() const
    {
        return name != nullptr && hasPriority;
    }

    void setPriority(double p)
    {
        priority = p;
        hasPriority = true;
    }

    bool operator > (const TestData &src) const
    {
        return priority > src.priority;
    }

    const char *name;
    bool hasPriority;
    double priority;
    m::List<testAPI::TestFunc> subTests;
};

typedef m::List<TestData> TestList;

static uint8_t *_g_testList[sizeof(TestList)];
#define g_testList reinterpret_cast<TestList*>(_g_testList)

M_DEFINE_NIFTY_COUNTER(TestAPI)
M_NIFTY_COUNTER_CTOR(TestAPI)
{
    new(g_testList) TestList;
}

M_NIFTY_COUNTER_DTOR(TestAPI)
{
    g_testList->~TestList();
}

void testAPI::declareName(const char *name)
{
    if(g_testList->isEmpty() || g_testList->last().isValid())
        g_testList->add(TestData(name));
    else
        g_testList->last().name = name;
}

void testAPI::declarePriority(double p)
{
    if(g_testList->isEmpty() || g_testList->last().isValid())
        g_testList->add(TestData(p));
    else
        g_testList->last().setPriority(p);
}

void testAPI::addFunc(TestFunc fc)
{
    g_testList->last().subTests.add(fc);
}

bool testAPI::runAll(const char *exeLoc, std::function<void(int, int, bool)> cb)
{
    g_testList->insertionSort();
    int total = 0, pos = 0;
    for(TestData &td: *g_testList)
        total += td.subTests.size();

    for(TestData &td: *g_testList) {
        m::console::setTextColor(m::kCC_Cyan);
        std::cout << "[i] Testing " << td.name << "..." << std::endl;
        m::console::resetColor();

        for(int i = 0; i < td.subTests.size(); i++) {
            if(td.subTests[i](exeLoc)) {
                m::console::setTextColor(m::kCC_Green);
                std::cout << "[i]\tTest #" << i + 1 << " succeeded!" << std::endl;
                m::console::resetColor();
                cb(++pos, total, true);
            } else {
                m::console::setTextColor(m::kCC_Red);
                std::cout << "[!]\tSubtest #" << i + 1 << " of category " << td.name << " failed!" << std::endl;
                m::console::resetColor();
                cb(++pos, total, false);
                return false;
            }
        }

        m::console::setTextColor(m::kCC_LightGreen);
        std::cout << "[i] Testing of " << td.name << " succeeded!" << std::endl;
        m::console::resetColor();
    }

    return true;
}

bool testAPI::testOnly(const char *tname)
{
    TestData backup;

    for(int i = 0; i < g_testList->size(); i++) {
        if(strcmp(g_testList->get(i).name, tname) == 0) {
            backup = g_testList->get(i);
            g_testList->clear();
            g_testList->add(backup);
            return false;
        }
    }

    //Print test list:
    std::cout << "Couldn't find test '" << tname << "'; here's a list:" << std::endl;
    g_testList->insertionSort();
    for(TestData &td : *g_testList)
        std::cout << '\t' << td.name << std::endl;

    return true;
}

void testAPI::testExcept(const char *tname)
{
    for(int i = 0; i < g_testList->size(); i++) {
        if(strcmp(g_testList->get(i).name, tname) == 0) {
            g_testList->remove(i);
            return;
        }
    }
}

#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

void testAPI::onAssertionFailed(const char *file, int line, const char *msg)
{
    m::console::setTextColor(m::kCC_Red);
    std::cout << "[!]\tAssertion failed in file \"" << file << "\" at line " << line << ':' << std::endl;
    std::cout << "[!]\t" << msg << std::endl;
    m::console::resetColor();

#if defined(MGPCL_WIN) && defined(_DEBUG)
    std::cout.flush(); //Make sure it has been written before we break

    std::ostringstream oss;
    oss << "Assertion failed in file \"" << file << "\" at line " << line << ':' << std::endl;
    oss << msg << std::endl;

    //Also output to VS console and trigger debugger
    OutputDebugString(oss.str().c_str());
    DebugBreak();
#endif
}
