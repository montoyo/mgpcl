#include "TestAPI.h"
#include <mgpcl/SharedPtr.h>
#include <mgpcl/BufferIOStream.h>
#include <mgpcl/DataIOStream.h>
#include <mgpcl/STDIOStream.h>
#include <mgpcl/TextIOStream.h>
#include <mgpcl/ByteBuf.h>
#include <mgpcl/SerialIO.h>
#include <mgpcl/Time.h>

Declare Test("io"), Priority(5.0);

TEST
{
    static StackIntegrityChecker sic;
    uint8_t *buf = new uint8_t[1024];

    for(int i = 0; i < 2; i++) {
        m::Endianness e;
        if(i == 0) {
            std::cout << "[i]\tTesting DataIO with little endian" << std::endl;
            e = m::Endianness::Little;
        } else {
            std::cout << "[i]\tTesting DataIO with big endian" << std::endl;
            e = m::Endianness::Big;
        }

        {
            m::SSharedPtr<m::DataOutputStream> dos(new m::DataOutputStream(new m::BufferOutputStream(buf, 1024)));
            dos->setEndianness(e);

            *dos << int(32) << uint16_t(12);
            *dos << m::String("some test"_m);
            *dos << float(12.14f);

            uint64_t pos = dos->pos();
            dos->seek(0);
            *dos << int(45);

            std::cout << "[i]\tWrote " << pos << " bytes." << std::endl;
        }

        {
            m::SSharedPtr<m::DataInputStream> dis(new m::DataInputStream(new m::BufferInputStream(buf, 1024)));
            dis->setEndianness(e);

            int a;
            uint16_t b;
            m::String c;
            float d;

            *dis >> a >> b >> c >> d;
            std::cout << "[i]\tRead " << dis->pos() << " bytes." << std::endl;

            testAssert(a == 45, "invalid int reading");
            testAssert(b == 12, "invalid short reading");
            testAssert(c == "some test", "invalid string reading");
            testAssert(d == 12.14f, "invalid float reading");
        }
    }

    delete[] buf;
    return true;
}

//#define TRACK_POS() std::cout << "[i]\t>>Line " << std::dec << __LINE__ << ", DOS::pos() == " << std::dec << dos->pos() << ", ByteBuf::size() == " << std::dec << bb.size() << std::endl

TEST
{
    volatile StackIntegrityChecker sic;
    m::ByteBuf bb;

    {
        m::SSharedPtr<m::DataOutputStream> dos(new m::DataOutputStream(bb.outputStream<m::RefCounter>()));
        dos->seek(6);
        *dos << int(-789);
        dos->seek(0);
        *dos << m::String("test"_m);
        dos->seek(0);
        *dos << uint16_t(3);
        testAssert(dos->seek(0, m::SeekPos::End), "seek failed");
        *dos << double(1.54);
    }

    std::cout << "[i]\tByteBuf::size() == " << std::dec << bb.size() << std::endl;
    testAssert(bb.size() == 18, "invalid dynamic buffer size");

    {
        m::SSharedPtr<m::DataInputStream> dis(new m::DataInputStream(bb.inputStream<m::RefCounter>()));
        m::String a;
        char b;
        int c;
        double d;

        *dis >> a >> b >> c >> d;

        testAssert(a == "tes"_m, "invalid string reading");
        testAssert(b == 't', "invalid char reading");
        testAssert(c == -789, "invalid int reading");
        testAssert(d == 1.54, "invalid double reading");
    }

    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;

    m::SSharedPtr<m::OutputStream> cout(new m::STDOutputStream(m::STDHandle::HOutput));
    m::TextOutputStream tos(cout);

    tos << "[i]\tthis is a test" << m::eol; //Test without m::String
    tos << "[i]\tSome pointer: "_m << &tos << m::eol;
    tos << "[i]\tSome integer: "_m << 1234 << m::eol;
    tos << "[i]\tSome double: "_m << 12.34 << m::eol;
    tos << "[i]\tSome char: "_m << 'c' << m::eol;
    tos << "[i]\tSome byte: "_m << static_cast<uint8_t>(0xab) << m::eol;

    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;

    m::List<m::String> devs;
    testAssert(m::SerialPort::listDevices(devs), "couldn't list serial devices");

    for(m::String &dev: devs)
        std::cout << "[i]\tFound serial device " << dev.raw() << std::endl;

    return true;
}

//Serial IO test. Disabled by default to avoid messing
//around with your serial devices. If you have any, that is.
#ifdef MGPCL_WIN
#define M_TEST_SERIAL_PORT "COM3"_m
#else
#define M_TEST_SERIAL_PORT "/dev/ttyACM0"_m
#endif

static bool readTO(m::TextInputStream &tis, m::String &dst)
{
    m::SerialInputStream &sis = *tis.child().staticCast<m::SerialInputStream>();
    double start = m::time::getTimeMs();

    while(sis.available() <= 0 && m::time::getTimeMs() - start < 5000.0)
        m::time::sleepMs(50);

    if(sis.available() <= 0)
        return false;

    dst.cleanup();
    tis.readLine(dst);
    return true;
}

DISABLED_TEST
{
    volatile StackIntegrityChecker sic;
    m::SerialPort sp;
    testAssert(sp.open(M_TEST_SERIAL_PORT, m::SerialPort::kAF_ReadWrite), "could not open serial port");
    sp.setArduinoConfig(m::SerialPort::kBR_9600);
    testAssert(sp.applyConfig(), "could not apply serial port config");

    m::TextInputStream tis(sp.inputStream<m::RefCounter>().staticCast<m::InputStream>());
    m::TextOutputStream tos(sp.outputStream<m::RefCounter>().staticCast<m::OutputStream>(), m::LineEnding::CRLF);
    m::String line;

    tos << "What's on this drive?"_m << m::eol;
    testAssert(readTO(tis, line), "arduino didn't answer 1");
    testAssert(line == "Project insight requires... insight"_m, "arduino didn't answer correctly 1");
    testAssert(readTO(tis, line), "arduino didn't answer 2");
    testAssert(line == "So I wrote an algorithm"_m, "arduino didn't answer correctly 2");
    tos << "What kind of algorithm? What does it do?"_m << m::eol;
    testAssert(readTO(tis, line), "arduino didn't answer 3");
    testAssert(line == "The answer to your question is fascinating."_m, "arduino didn't answer correctly 3");
    testAssert(readTO(tis, line), "arduino didn't answer 4");
    testAssert(line == "Unfortunately, you shall be too dead to hear it."_m, "arduino didn't answer correctly 4");

    tis.close();
    tos.close();
    sp.close();
    return true;
};
