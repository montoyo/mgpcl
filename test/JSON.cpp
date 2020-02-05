#include "TestAPI.h"
#include <mgpcl/Time.h>
#include <mgpcl/File.h>
#include <mgpcl/JSON.h>
#include <mgpcl/FileIOStream.h>
#include <mgpcl/StringIOStream.h>

Declare Test("json"), Priority(7.0);

TEST
{
    volatile StackIntegrityChecker sic;
    m::SSharedPtr<m::InputStream> is(new m::FileInputStream("test.json"_m));
    m::String err;
    m::JSONElement root;

    if(!m::json::parse(is, root, err)) {
        std::cout << "[!]\t" << err.raw() << std::endl;
        return false;
    }

    m::JSONElement &sa = root["someArray"_m];
    testAssert(sa.isArray(), "'someArray' is not an array");
    
    m::JSONElement &saLast = sa[sa.size() - 1];
    testAssert(saLast.isObject(), "'someArray.last()' is not an object");

    m::JSONElement &dee = saLast["dee"_m];
    testAssert(dee.isString(), "'dee' is not a string");
    testAssert(dee.asString() == "hel\tlo"_m, "'dee' has an unexpected value");

    m::SSharedPtr<m::StringOStream> sos(new m::StringOStream);
    testAssert(m::json::serializeCompact(sos.staticCast<m::OutputStream>(), root), "couldn't serialize JSON 1");
    std::cout << "[i]\tCompact data is:" << sos->data().raw() << std::endl;

    sos->cleanup();
    testAssert(m::json::serializeHumanReadable(sos.staticCast<m::OutputStream>(), root), "couldn't serialize JSON 2");
    std::cout << "[i]\tHuman data is:\n" << sos->data().raw() << std::endl;

    return true;
}

DISABLED_TEST
{
    volatile StackIntegrityChecker sic;
    m::File citylots("citylots.json"_m);

    if(citylots.exists()) {
        m::SSharedPtr<m::InputStream> is(new m::FileInputStream("citylots.json"_m));
        m::String err;
        m::JSONElement root;

        double start = m::time::getTimeMs();
        if(!m::json::parse(is, root, err)) {
            std::cout << "[!]\t" << err.raw() << std::endl;
            return false;
        }

        double t = m::time::getTimeMs() - start;
        std::cout << "[i]\tFinished parsing in " << t << " ms!" << std::endl;
    } else
        std::cout << "[i]\tHuge citylots.json test file is missing; not running test..." << std::endl;

    return true;
}
