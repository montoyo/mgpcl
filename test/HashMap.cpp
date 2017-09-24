#include "TestAPI.h"
#include <mgpcl/HashMap.h>
#include <mgpcl/FlatMap.h>
#include <mgpcl/String.h>

Declare Test("hashMap"), Priority(3);

TEST
{
    volatile StackIntegrityChecker sic;
    m::HashMap<m::String, TestObject> map;

    map["abcdef"] = TestObject();
    map.get("ghijkl") = TestObject(0xBADC0FFE);

    testAssert(map.size() == 2, "invalid map size 1");
    testAssert(!map.hasKey("abcjkl"), "found key \"abcjkl\", which shouldn't exist...");
    testAssert(map.hasKey("abcdef"), "could not found key \"abcdef\"...");

    std::cout << "[i]\tTesting direct order..." << std::endl;
    for(m::HashMap<m::String, TestObject>::Pair &p : map)
        std::cout << "[i]\tmap[\"" << p.key.raw() << "\"] = TestObject(0x" << std::hex << std::uppercase << p.value.value() << ")" << std::endl;

    std::cout << std::endl << "[i]\tTesting reverse order..." << std::endl;
    m::HashMap<m::String, TestObject>::Iterator it(map.end());
    do {
        --it;
        std::cout << "[i]\tmap[\"" << it->key.raw() << "\"] = TestObject(0x" << std::hex << std::uppercase << it->value.value() << ")" << std::endl;
    } while(it != map.begin());

    std::cout << std::endl;
    
    map.removeKey("abcdef");
    testAssert(map.size() == 1, "invalid map size 2");
    testAssert(!map.hasKey("abcdef"), "found key \"abcdef\", which has been removed...");
    testAssert(map.hasKey("ghijkl"), "could not found key \"ghijkl\"...");

    map.clear();
    testAssert(TestObject::instances() == 0, "some instances of TestObject are STILL alive!!");
    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;
    m::FlatMap<int, m::String> map;

    for(int i = 0; i < 16; i++)
        map[i * 128] = m::String::fromInteger(i);

    const int tIdx = 4 * 128 + 42;
    map.put(tIdx, "I like trains");

    bool hadSpecial = false;
    int prevIdx = -1;

    for(m::FlatMap<int, m::String>::Pair &elem : map) {
        testAssert(elem.key > prevIdx, "indexes are in the wrong order");
        prevIdx = elem.key;

        if(elem.key == tIdx)
            hadSpecial = true;
        else
            testAssert(elem.value == m::String::fromInteger(elem.key / 128), "value doesn't match key");
    }

    testAssert(hadSpecial, "couldn't find special value");

    //Check they all have their indexes
    for(int i = 0; i < 16; i++)
        testAssert(map.hasKey(i * 128), "missing index in map");

    testAssert(map.hasKey(tIdx), "missing special index in map");

    //Check we can access everything
    for(int i = 0; i < 16; i++)
        testAssert(map[i * 128] == m::String::fromInteger(i), "missing value 1");

    testAssert(map[tIdx] == "I like trains", "missing value 2");
    testAssert(map.removeKey(tIdx), "couldn't remove key");
    testAssert(!map.hasKey(tIdx), "key deletion failed");

    return true;
}

//Tweeted by @Snowden (SHA-2?) ffdae96f8dd292374a966ec8b57d9cc680ce1d23cb7072c522efe32a1a7e34b0

