#include "TestAPI.h"
#include <mgpcl/BasicLogger.h>
#include <mgpcl/NetLogger.h>
#include <mgpcl/Time.h>

Declare Test("logging"), Priority(12.0);

static void testLogger(const char *threadName)
{
	mlogger.info(M_LOG, "Hello %s!", "world");
	mlogger.debug(M_LOG, "This is a debug message: %p", m::Logger::instance());
	mlogger.warning(M_LOG, "This is a warning message!");
	mlogger.error(M_LOG, "This is an error message!");

	m::FunctionalThread ft([] () {
		mlogger.info(M_LOG, "Hello %f!", 92.4);
	}, threadName);

	ft.start();
	m::time::sleepMs(1000);
	ft.join();
}

TEST
{
	volatile StackIntegrityChecker sic;
	testAssert(m::Logger::instance() == nullptr, "found an old logger");

	std::cout.flush(); //Color fix!

	m::Logger::setLoggerInstance(new m::BasicLogger);
	testLogger("LONGTHREADNAME");
	delete m::Logger::setLoggerInstance(nullptr);

	return true;
}

TEST
{
	volatile StackIntegrityChecker sic;
	testAssert(m::Logger::instance() == nullptr, "found an old logger");

	m::NetLogger *logger = new m::NetLogger;
	if(!logger->tryAutoStartSubprocess())
		std::cout << "[W]\tCould not start subprocess" << std::endl;

	m::Logger::setLoggerInstance(logger);
	if(logger->connect("127.0.0.1:1234")) {
		testLogger("TEST");
		logger->debug(M_LOG, "Pointer 2: %p", &sic);

		m::time::sleepMs(250);
		logger->disconnect();
	} else
		std::cout << "[W]\tCould not connect: assuming server is offline..." << std::endl;

	delete m::Logger::setLoggerInstance(nullptr);
	return true;
}
