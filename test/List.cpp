#include "TestAPI.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <mgpcl/List.h>
#include <mgpcl/Queue.h>
#include <mgpcl/Time.h>

Declare Test("lists"), Priority(1.0);

TEST
{
    volatile StackIntegrityChecker sic;
    m::List<TestObject> testA{ TestObject(123401), TestObject(123402), TestObject(123403), TestObject(123404) };
    m::List<TestObject> testB;
    m::List<TestObject> testC(testA.sub(0, 2));

    testB.add(TestObject(123401));
    testB.add(TestObject(123404));
    testB.add(TestObject(123402));
    testB.add(TestObject(123403));

    m::List<TestObject> testD(testB);

    {
        TestObject to[] = { TestObject(123404), TestObject(123403) };
        testC.addAll(to, 2);
        testC.remove(testC.size() - 2);
        
        testB.insertionSort([] (const TestObject &a, const TestObject &b) -> bool {
            return a.value() > b.value();
        });

        testC.add(testB.last());
    }

    {
        TestObject to[] = { TestObject(123403), TestObject(123404) };
        testD.insert(1, TestObject(123402));
        testD.replaceAll(2, to, 2);
        testD.remove(4);
    }

    testAssert(testA.size() == 4, "invalid size for List A");
    testAssert(testB.size() == 4, "invalid size for List B");
    testAssert(testC.size() == 4, "invalid size for List C");
    testAssert(testD.size() == 4, "invalid size for List D");

    m::List<m::List<TestObject>*> all;
    all << &testA << &testB << &testC << &testD;

    testAssert(all.size() == 4, "invalid size for List \"all\"");
    testAssert(all[0] == &testA, "invalid value for all[0]");
    testAssert(all[1] == &testB, "invalid value for all[1]");
    testAssert(all[2] == &testC, "invalid value for all[2]");
    testAssert(all[3] == &testD, "invalid value for all[3]");

    while(all.size() > 0) {
        m::List<TestObject> *ptr;
        all >> ptr;

        int i = 0;
        for(TestObject &o : *ptr) {
            testAssert(o.value() == 123401 + i, "invalid value for some list");
            i++;
        }
    }

    std::cout << "[i]\tProceeding to list cleanup..." << std::endl;
    testA.clear();
    testB.cleanup();
    testC.clear();
    testD.cleanup();

    testAssert(TestObject::instances() == 0, "invalid test object count");
    return true;
}

class TestNumber
{
public:
    TestNumber()
    {
        m_val = rand() % 10000 - 5000;
    }

    int value() const
    {
        return m_val;
    }

    bool operator >= (const TestNumber &tn) const
    {
        return m_val >= tn.m_val;
    }

    bool operator > (const TestNumber &tn) const
    {
        return m_val > tn.m_val;
    }

private:
    int m_val;
};

TEST
{
    volatile StackIntegrityChecker sic;
    m::List<int> lst;

    srand(static_cast<unsigned int>(time(nullptr)));
    for(int i = 0; i < 20000; i++)
        lst << rand() % 10000 - 5000;

    m::time::initTime();
    double s = m::time::getTimeMs();
    lst.insertionSort();
    std::cout << "[i]\tInsertion sort (ints) took " << m::time::getTimeMs() - s << "ms" << std::endl;

    for(int i = 0; i < lst.size() - 1; i++)
        testAssert(lst[i] <= lst[i + 1], "list sorting failed");

    lst.clear();
    m::List<TestNumber> lst2;
    for(int i = 0; i < 20000; i++)
        lst2 << TestNumber();

    s = m::time::getTimeMs();
    lst2.insertionSort();
    std::cout << "[i]\tInsertion sort (TestNumbers) took " << m::time::getTimeMs() - s << "ms" << std::endl;

    for(int i = 0; i < lst2.size() - 1; i++)
        testAssert(lst2[i].value() <= lst2[i + 1].value(), "list sorting failed");

    lst2.clear();
    m::List<float> lst3;

    for(int i = 0; i < 20000; i++) {
        float a = static_cast<float>(rand() % 10000 - 5000);
        float b = static_cast<float>(rand() % 10000 - 5000) / 10000.f;

        lst3 << (a + b);
    }

    s = m::time::getTimeMs();
    lst3.insertionSort();
    std::cout << "[i]\tInsertion sort (floats) took " << m::time::getTimeMs() - s << "ms" << std::endl;

    for(int i = 0; i < lst3.size() - 1; i++)
        testAssert(lst3[i] <= lst3[i + 1], "list sorting failed");

    lst3.clear();
    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;
    m::Queue<TestObject> q(8);

    q.offer(TestObject(0xABCDEFFF));
    q.offer(TestObject(0xABCDEF01));
    q.offer(TestObject(0xABCDEF02));
    q.offer(TestObject(0xABCDEF03));
    q.offer(TestObject(0xABCDEF04));
    q.poll();
    q.offer(TestObject(0xABCDEF05));
    q.offer(TestObject(0xABCDEF06));
    q.offer(TestObject(0xABCDEF07));
    q.offer(TestObject(0xABCDEF08));

    for(int i = 0; i < 8; i++)
        q.poll();

    q.offer(TestObject(0x11223344));
    q.clear();

    testAssert(TestObject::instances() == 0, "invalid test object count");
    return true;
}

static const int g_12345[] = { 1, 2, 3, 4, 5 };

static void genArray(int *sz, int *dst)
{
    if(dst == nullptr) {
        *sz = 5;
        return;
    } else if(*sz != 5)
        abort();

    m::mem::copy(dst, g_12345, 5 * sizeof(int));
}

class ClassWithEqualOp
{
public:
    bool operator == (const ClassWithEqualOp &test) const
    {
        return true;
    }
};

class ClassWithoutEqualOp
{
};

TEST
{
    volatile StackIntegrityChecker sic;
    m::List<int> test;
    testAssert(test.setFromDoubleCall([] (int *sz, int *dst) -> bool {
        genArray(sz, dst);
        return true;
    }), "List<int>::setFromDoubleCall failed");

    /*m::List<TestObject> invalidTest;
    invalidTest.setFromDoubleCall([] (int*, TestObject*) -> bool { return false; });*/

    testAssert(~test == 5, "setFromDoubleCall got bad size");
    for(int i = 0; i < 5; i++)
        testAssert(test[i] == i + 1, "setFromDoubleCall copied data wrongly!");

    m::List<int> anotherTest{ 1, 2, 3, 4, 5 };
    testAssert(test == anotherTest, "m::List::operator == doesn't work with ints");

    m::List<ClassWithEqualOp> lst1;
    lst1 << ClassWithEqualOp();
    lst1 << ClassWithEqualOp();

    decltype(lst1) lst2(lst1);
    testAssert(lst1 == lst2, "m::List::operator == doesn't work with classes");

    /*
    m::List<ClassWithoutEqualOp> lst3;
    lst3 << ClassWithoutEqualOp();
    lst3 << ClassWithoutEqualOp();

    decltype(lst3) lst4(lst3);
    std::cout << (lst3 == lst4) << std::endl;
    */

    return true;
};
