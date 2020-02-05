#include "TestAPI.h"
#include <sstream>
#include <mgpcl/Pattern.h>
#include <mgpcl/Date.h>
#include <mgpcl/CPUInfo.h>
#include <mgpcl/UUID.h>
#include <mgpcl/Random.h>

Declare Test("strings"), Priority(2.0);

TEST
{
    volatile StackIntegrityChecker sic;
    m::String test("i love dicks"_m);
    test = test.substr(0, -1);
    test = test.substr(0, -6);
    test += "you";

    std::cout << "[i]\tGot string: \"" << test.raw() << "\", expecting \"i love you\"" << std::endl;
    testAssert(test == "i love you", "comparaison #1 failed!");
    testAssert(test.upper().hash() == "I LOVE YOU"_m.hash(), "comparaison #2 failed!");

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
    str3 = "some test\n \r"_m;

    test = test.trimmed();
    str2 = str2.trimmedLeft();
    str3 = str3.trimmedRight();

    std::cout << "[i]\tStrings \"" << test.raw() << "\", \"" << str2.raw() << "\" and \"" << str3.raw() << "\" should match!" << std::endl;
    testAssert(test == "some test"_m, "trimming failed");
    testAssert(test == str2, "strings should match");
    testAssert(str2 == str3, "strings should match");
    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;
    testAssert(m::String("1234").isInteger() && m::String("1234").isNumber(), "expected 1234 to be an int and a number");
    testAssert(!m::String("12.34").isInteger() && m::String("12.34"_m).isNumber(), "expected 12.34 to be a number");
    testAssert(!m::String(".1234").isInteger() && m::String(".1234").isNumber(), "expected .1234 to be a number");
    testAssert(!m::String("1234."_m).isInteger() && m::String("1234.").isNumber(), "expected 1234. to be an number");
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
    testAssert(!test.parseRFC6265_511("Wed, 09 Jan 10:18:14 GMT"_m), "date parsing should have failed 4");
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

    m::String t1("i like trains"_m);
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
    testAssert(t3.erase(6) == "i like"_m, "take failed (end)");

    m::String tmp;
    {
        makeSizeString(1, tmp);
        testAssert(tmp == "1 byte"_m, "invalid size parsing 1");
    }

    {
        tmp.cleanup();
        makeSizeString(5, tmp);
        testAssert(tmp == "5 bytes"_m, "invalid size parsing 2");
    }

    {
        tmp.cleanup();
        makeSizeString(1024, tmp);
        testAssert(tmp == "1 KiB"_m, "invalid size parsing 3");
    }

    {
        tmp.cleanup();
        makeSizeString(1024 * 1024, tmp);
        testAssert(tmp == "1 MiB"_m, "invalid size parsing 4");
    }

    {
        tmp.cleanup();
        makeSizeString(1024 * 1024 * 1024, tmp);
        testAssert(tmp == "1 GiB"_m, "invalid size parsing 5");
    }

    {
        tmp.cleanup();
        makeSizeString(1ULL << 40ULL, tmp);
        testAssert(tmp == "1 TiB"_m, "invalid size parsing 6");
    }

    {
        tmp.cleanup();
        makeSizeString(1ULL << 50ULL, tmp);
        testAssert(tmp == "1024 TiB"_m, "invalid size parsing 7");
    }

    {
        tmp.cleanup();
        makeSizeString(1024 + 256, tmp);
        testAssert(tmp == "1.25 KiB"_m, "invalid size parsing 8");
    }

    return true;
}

TEST
{
    //I should probably move this to Misc

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

TEST
{
    volatile StackIntegrityChecker sic;
    static const uint8_t origHex[] = { 0x00, 0x11, 0xAF, 0x33, 0x44, 0x55, 0x66, 0x77 };

    uint8_t testBuf[8];
    const uint32_t testSz = 8;

    const m::String test1("this isn't valid"_m);
    const m::String test2("abcabcabcabcabc"_m);
    const m::String test3("0011aF3344556677"_m);

    testAssert(m::unHexString(test1, testBuf, testSz) == 0, "un-hex test1 should have failed");
    testAssert(m::unHexString(test2, testBuf, testSz) == 0, "un-hex test2 should have failed");
    testAssert(m::unHexString(test3, testBuf, testSz) == 8, "un-hex test3 failed for no reason");
    testAssert(m::mem::cmp(testBuf, origHex, 8) == 0, "un-hexed data does not match hex data");

    return true;
}

#ifdef MGPCL_ENABLE_PATTERNS

TEST
{
    volatile StackIntegrityChecker sic;

    {
        m::Pattern pat;
        testAssert(!pat.compile("te(st"), "compilation of pattern #1 should have failed");
        testAssert(pat.parseError() == m::kPPE_UnclosedParenthesis, "wrong parse error for pattern #1");

        testAssert(!pat.compile("te[st"), "compilation of pattern #2 should have failed");
        testAssert(pat.parseError() == m::kPPE_UnclosedBracket, "wrong parse error for pattern #2");

        testAssert(!pat.compile("te[T-S]"), "compilation of pattern #3 should have failed");
        testAssert(pat.parseError() == m::kPPE_InvalidRange, "wrong parse error for pattern #3");

        testAssert(!pat.compile("te()st"), "compilation of pattern #4 should have failed");
        testAssert(pat.parseError() == m::kPPE_EmptyCapture, "wrong parse error for pattern #4");

        testAssert(!pat.compile("+test"), "compilation of pattern #5 should have failed");
        testAssert(pat.parseError() == m::kPPE_MisplacedCtrlChar, "wrong parse error for pattern #5");

        testAssert(!pat.compile("te[s*]+t"), "compilation of pattern #6 should have failed");
        testAssert(pat.parseError() == m::kPPE_MisplacedCtrlChar, "wrong parse error for pattern #6");

        testAssert(!pat.compile("te[A-$]+t"), "compilation of pattern #7 should have failed");
        testAssert(pat.parseError() == m::kPPE_MisplacedCtrlChar, "wrong parse error for pattern #7");

        testAssert(!pat.compile(""), "compilation of pattern #8 should have failed");
        testAssert(pat.parseError() == m::kPPE_EmptyPattern, "wrong parse error for pattern #8");

        testAssert(!pat.compile("^$"), "compilation of pattern #9 should have failed");
        testAssert(pat.parseError() == m::kPPE_EmptyPattern, "wrong parse error for pattern #9");
    }

    {
        m::Pattern pat;
        testAssert(pat.compile("^a(bb)+a$"), "could not compile pattern #1");
        testAssert(pat != "zdf"   , "pattern #1 test #1 shouldn't match");
        testAssert(pat != "ab"    , "pattern #1 test #2 shouldn't match");
        testAssert(pat != "aba"   , "pattern #1 test #3 shouldn't match");
        testAssert(pat == "abba"  , "pattern #1 test #4 should match");
        testAssert(pat != "abbba" , "pattern #1 test #5 shouldn't match");
        testAssert(pat == "abbbba", "pattern #1 test #6 should match");
    }

    {
        m::Pattern pat;
        testAssert(pat.compile("^[E-T]+ *=%s*[ste]+$"), "could not compile pattern #2");
        testAssert(pat != "zdf"          , "pattern #2 test #1 shouldn't match");
        testAssert(pat != "="            , "pattern #2 test #2 shouldn't match");
        testAssert(pat != "test = test"  , "pattern #2 test #3 shouldn't match");
        testAssert(pat == "TEST = test"  , "pattern #2 test #4 should match");
        testAssert(pat == "TEST=test"    , "pattern #2 test #5 should match");
        testAssert(pat == "TEST   = test", "pattern #2 test #6 should match");
        testAssert(pat == "TEST= \t test", "pattern #2 test #7 should match");
    }

    {
        m::Pattern pat;
        testAssert(pat.compile("%d%d/%d%d/%d%d%d%d"), "could not compile pattern #3");
        testAssert(!pat.matcher("zdf").next(), "pattern #3 test #1 shouldn't match");

        m::Matcher matcher(pat.matcher("The date is 01/11/2024 lol"));
        testAssert(matcher.next(), "pattern #3 test #2 should match");
        testAssert(matcher.capture() == "01/11/2024", "pattern #3 test #2 captured match is wrong");
    }

    return true;
}

#endif

TEST
{
    volatile StackIntegrityChecker sic;
    m::prng::Xoroshiro x;

    for(int i = 0; i < 8; i++) {
        m::UUID test(x);
        m::String uuidStr(test.toString());

        std::cout << "[i]\tUUID is " << uuidStr.raw() << std::endl;
        testAssert(uuidStr.length() == 36, "failed to convert UUID to string");
        testAssert(uuidStr[14] == '4', "invalid UUID version");
        testAssert(uuidStr[19] == '8' || uuidStr[19] == '9' || uuidStr[19] == 'a' || uuidStr[19] == 'b', "invalid UUID version");

        m::UUID test2(uuidStr, true);
        testAssert(test == test2, "failed to re-parse UUID (strict = true)");

        m::UUID test3(uuidStr, false);
        testAssert(test == test3, "failed to re-parse UUID (strict = false)");
    }

    m::UUID nilTest(x);
    testAssert(nilTest.setFromString("00000000-0000-0000-0000-000000000000"_m, true), "parsing nil shouldn't have failed (strict = true)");
    testAssert(nilTest.isNil(), "parsing nil failed (strict = true)");
    nilTest.regenerate(x);
    testAssert(nilTest.setFromString("0-0-0-0-0"), "parsing nil shouldn't have failed (strict = false)");
    testAssert(nilTest.isNil(), "parsing nil failed (strict = false)");

    return true;
}
