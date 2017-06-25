#include "TestAPI.h"
#include <sstream>
#include <mgpcl/String.h>
#include <mgpcl/Date.h>
#include <mgpcl/CPUInfo.h>

Declare Test("strings"), Priority(2.0);

TEST
{
	volatile StackIntegrityChecker sic;
    m::String test("i love dicks");
    test = test.substr(0, -1);
    test = test.substr(0, -6);
    test += "you";

    std::cout << "[i]\tGot string: \"" << test.raw() << "\", expecting \"i love you\"" << std::endl;
    testAssert(test == "i love you", "comparaison #1 failed!");
    testAssert(test.upper().hash() == m::String("I LOVE YOU").hash(), "comparaison #2 failed!");

    m::String str2;
    str2 += test.substr(0, 2);
    str2 += "hate";
    str2 += test.substr(test.length() - 4);

    m::List<m::String> data;
    test.splitOnOneOf(" \t\r\n", data);
    testAssert(data.size() == 3, "expected 3 values in list");

    m::String str3(data[0]);
    str3 += ' ';
    str3 += "hate";
    str3 += ' ';
    str3 += data[2];

    std::cout << "[i]\tStrings \"" << str2.raw() << "\" and \"" << str3.raw() << "\" should match!" << std::endl;
    testAssert(str2 == str3, "strings should match");

    test = "      \r    some test ";
    str2 = "\t   some test";
    str3 = "some test\n \r";

    test = test.trimmed();
    str2 = str2.trimmedLeft();
    str3 = str3.trimmedRight();

    std::cout << "[i]\tStrings \"" << test.raw() << "\", \"" << str2.raw() << "\" and \"" << str3.raw() << "\" should match!" << std::endl;
    testAssert(test == "some test", "trimming failed");
    testAssert(test == str2, "strings should match");
    testAssert(str2 == str3, "strings should match");
	return true;
}

TEST
{
    volatile StackIntegrityChecker sic;
    testAssert(m::String("1234").isInteger() && m::String("1234").isNumber(), "expected 1234 to be an int and a number");
    testAssert(!m::String("12.34").isInteger() && m::String("12.34").isNumber(), "expected 12.34 to be a number");
    testAssert(!m::String(".1234").isInteger() && m::String(".1234").isNumber(), "expected .1234 to be a number");
    testAssert(!m::String("1234.").isInteger() && m::String("1234.").isNumber(), "expected 1234. to be an number");
    testAssert(!m::String("a456").isInteger() && !m::String("a456").isNumber(), "didn't expect a456 to be a number/int");
    testAssert(m::String("COFFEE").isInteger(16), "expected COFFEE to be an int");
    testAssert(m::String("1234").isInteger() && m::String("1234").isNumber(), "expected 1234 to be an int and a number");
    testAssert(m::String::fromInteger(0) == "0", "wrong zero!");
    testAssert(m::String::fromInteger(-124).toInteger() == -124, "-124 could not be converted");
    testAssert(m::String::fromInteger(789).toInteger() == 789, "789 could not be converted");
    testAssert(m::String::fromInteger(0xBEEF, 16).toInteger(16) == 0xBEEF, "0xBEEF could not be converted");

    m::List<double> toTest{ 0.47, 78485.588, 1.1548, 8789789.1237, -47.32 };
    toTest.insertionSort();

    for(double d : toTest) {
        std::ostringstream oss;
        oss.precision(4);
        oss << std::fixed << d;

        std::string a(oss.str());
        m::String b(m::String::fromDouble(d, 4));
        double dd = b.toDouble();

        std::cout << "[i]\tComparing strings std::\"" << a << "\" and m::\"" << b.raw() << "\" (reconv. ";
        std::cout.precision(4);
        std::cout << std::fixed << dd << ")" << std::endl;
        testAssert(b == a.c_str(), "bad double formatting");
        testAssert(dd > d - 0.00001 && dd < d + 0.00001, "bad double re-conversion");
    }

    m::String test(m::String::format("[%c]\t\"%s\" %d %f", 'i', "ZIS IZ A TEZT", 789, 42.69).toLower());
    std::cout << test.raw() << std::endl;
    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;
    m::Date test(1881631921);
    m::String d(test.format("%D/%O/%y %H:%M:%S"));
    std::cout << "[i]\tDate for 1881631921 is \"" << d.raw() << "\"" << std::endl;

    test = m::Date::now();
    d = test.format("%W %D %N %y, %H:%M:%S");
    std::cout << "[i]\tDate 1 for now is \"" << d.raw() << "\"" << std::endl;

    test = m::Date::now();
    d = test.format("%w %d %n %y, %h:%m:%s");
    std::cout << "[i]\tDate 2 for now is \"" << d.raw() << "\"" << std::endl;

	testAssert(!test.parseRFC6265_511("rzr45484,  /  rezrr re++"), "date parsing should have failed 1");
	testAssert(!test.parseRFC6265_511("Wed, 09 Jbn 2021 10:18:14 GMT"), "date parsing should have failed 2");
	testAssert(!test.parseRFC6265_511("Wed, Jan 2021 10:18:14 GMT"), "date parsing should have failed 3");
	testAssert(!test.parseRFC6265_511("Wed, 09 Jan 10:18:14 GMT"), "date parsing should have failed 4");
	testAssert(!test.parseRFC6265_511("Wed, 09 Jbn 2021 10:1x:14 GMT"), "date parsing should have failed 5");

	test = m::Date(); //Clear to be sure...
	testAssert(test.parseRFC6265_511("Wed, 09 Jan 2021 10:18:14 GMT"), "date parsing should be ok");

	testAssert(test.monthDay() == 9, "invalid month day");
	testAssert(test.month() == 1, "invalid month");
	testAssert(test.year() == 2021, "invalid year");
	testAssert(test.hour() == 10, "invalid hour");
	testAssert(test.minutes() == 18, "invalid minute");
	testAssert(test.seconds() == 14, "invalid seconds");

	testAssert(test.asUnixTimeGMT() == 1610187494, "invalid epoch");
    return true;
}

TEST
{
	volatile StackIntegrityChecker sic;

	m::String t1("i like trains");
	m::String t2(t1);
	m::String t3(t1);

	testAssert(t2.take(0, 2) == "i ", "take failed (beg)");
	testAssert(t3.erase(0, 2) == "like trains", "take failed (beg)");

	t2 = t1;
	t3 = t1;
	testAssert(t2.take(2, 7) == "like ", "take failed (mid)");
	testAssert(t3.erase(2, 7) == "i trains", "take failed (mid)");

	t2 = t1;
	t3 = t1;
	testAssert(t2.take(6) == " trains", "take failed (end)");
	testAssert(t3.erase(6) == "i like", "take failed (end)");

	m::String tmp;
	{
		makeSizeString(1, tmp);
		testAssert(tmp == "1 byte", "invalid size parsing 1");
	}

	{
		tmp.cleanup();
		makeSizeString(5, tmp);
		testAssert(tmp == "5 bytes", "invalid size parsing 2");
	}

	{
		tmp.cleanup();
		makeSizeString(1024, tmp);
		testAssert(tmp == "1 KiB", "invalid size parsing 3");
	}

	{
		tmp.cleanup();
		makeSizeString(1024 * 1024, tmp);
		testAssert(tmp == "1 MiB", "invalid size parsing 4");
	}

	{
		tmp.cleanup();
		makeSizeString(1024 * 1024 * 1024, tmp);
		testAssert(tmp == "1 GiB", "invalid size parsing 5");
	}

	{
		tmp.cleanup();
		makeSizeString(1ULL << 40ULL, tmp);
		testAssert(tmp == "1 TiB", "invalid size parsing 6");
	}

	{
		tmp.cleanup();
		makeSizeString(1ULL << 50ULL, tmp);
		testAssert(tmp == "1024 TiB", "invalid size parsing 7");
	}

	{
		tmp.cleanup();
		makeSizeString(1024 + 256, tmp);
		testAssert(tmp == "1.25 KiB", "invalid size parsing 8");
	}

	return true;
}

TEST
{
	volatile StackIntegrityChecker sic;
	m::CPUInfo info(m::CPUInfo::fetch());

	if(!info.isValid())
		std::cout << "[!]\tCouldn't fetch CPU info: " << info.error().raw() << std::endl;

	testAssert(info.isValid(), "couldn't fetch CPU info!");

	std::cout << "[i]\tCPUInfo::name() = " << info.name().raw() << std::endl;
	std::cout << "[i]\tCPUInfo::vendor() = " << info.vendor().raw() << std::endl;
	std::cout << "[i]\tCPUInfo::numCores() = " << info.numCores() << std::endl;
	std::cout << "[i]\tCPUInfo::maxFrequency() = " << info.maxFrequency() << " MHz" << std::endl;
	return true;
}