#pragma once
#include <mgpcl/NiftyCounter.h>
#include <iostream>
#include <functional>
#include "StackIntegrityChecker.h"
#include "TestObject.h"

#define testAssert(cond, msg) if(!(cond)) { \
	::testAPI::onAssertionFailed(__FILE__, __LINE__, msg); \
	return false; \
}

M_DECLARE_NIFTY_COUNTER(TestAPI)
#ifdef M_TESTAPI_SRC
#define M_TESTAPI_PREFIX
#else
M_SPAWN_NIFTY_COUNTER(TestAPI)
#define M_TESTAPI_PREFIX extern
#endif

namespace testAPI
{
	typedef bool(*TestFunc)(const char*);
	M_TESTAPI_PREFIX void declareName(const char *name);
	M_TESTAPI_PREFIX void declarePriority(double p);
	M_TESTAPI_PREFIX void addFunc(TestFunc fc);
	M_TESTAPI_PREFIX bool runAll(const char *exeLoc, std::function<void(int, int, bool)> cb);
	M_TESTAPI_PREFIX void onAssertionFailed(const char *file, int line, const char *msg);
	M_TESTAPI_PREFIX bool testOnly(const char *tname);
	M_TESTAPI_PREFIX void testExcept(const char *tname);
}

class _Declare
{
public:
	_Declare(const char *name)
	{
		testAPI::declareName(name);
	}

	_Declare(double p)
	{
		testAPI::declarePriority(p);
	}

	_Declare(testAPI::TestFunc fc)
	{
		testAPI::addFunc(fc);
	}

private:
	_Declare()
	{
	}
};

#define Declare static _Declare
#define _MAKEFID(id) _tf_ ## id
#define _MAKEOID(id) _to_ ## id
#define MAKEFID(id) _MAKEFID(id)
#define MAKEOID(id) _MAKEOID(id)
#define TEST static bool MAKEFID(__LINE__)(const char *exeLoc); Declare MAKEOID(__LINE__)(MAKEFID(__LINE__)); static bool MAKEFID(__LINE__)(const char *exeLoc)
#define DISABLED_TEST static bool MAKEFID(__LINE__)(const char *exeLoc); static bool MAKEFID(__LINE__)(const char *exeLoc)
