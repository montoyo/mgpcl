#include "TestAPI.h"
#include <mgpcl/HTTPRequest.h>
#include <mgpcl/StringIOStream.h>
#include <mgpcl/FileIOStream.h>
#include <mgpcl/Util.h>
#include <mgpcl/TCPClient.h>
#include <mgpcl/TCPServer.h>
#include <mgpcl/Time.h>

Declare Test("net"), Priority(10.0);

TEST
{
    volatile StackIntegrityChecker sic;

    m::IPv4Address test;
    m::IPv4Address r1(62, 210, 125, 206, 80);
    m::IPv4Address r2, r3;

    testAssert(test.resolve("montoyo.net"_m, 80) == m::kRE_NoError, "Failed to resolve montoyo.net");
    testAssert(test.addr(0) == 62 && test.addr(1) == 210 && test.addr(2) == 125 && test.addr(3) == 206, "Invalid address for montoyo.net");
    testAssert(test.port() == 80, "Invalid port for montoyo.net");
    testAssert(r2.parse("62.210.125.206:80"_m) == m::kAFE_NoError, "Could not parse addr from string 1");
    testAssert(r3.parse("62.210.125.206"_m, 80) == m::kAFE_NoError, "Could not parse addr from string 2");
    testAssert(test == r1, "Addresses do not match 1");
    testAssert(test == r2, "Addresses do not match 2");
    testAssert(test == r3, "Addresses do not match 3");
    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;

    m::IPv4Address addr;
    testAssert(addr.resolve("montoyo.net"_m, 80) == m::kRE_NoError, "Could not resolve montoyo.net");

    m::TCPSocket sock;
    testAssert(sock.initialize(), "Could not create socket");
    testAssert(sock.connect(addr) == m::kSCE_NoError, "Could not connect socket");

    m::String request("GET /ShareX/test HTTP/1.1\r\nHost: montoyo.net\r\nUser-Agent: Gaben\r\n\r\n"_m);
    testAssert(sock.send(reinterpret_cast<const uint8_t*>(request.raw()), request.length()) == request.length(), "Could not send HTTP request");

    char buf[8192];
    int read = sock.receive(reinterpret_cast<uint8_t*>(buf), 8192);
    testAssert(read > 0, "Could not receive HTTP request");

    m::String response;
    response.append(buf, read);

    m::List<m::String> lines;
    response.splitOn("\r\n", lines, 2);

    if(!lines.isEmpty()) {
        if(lines.last().isEmpty())
            lines.remove(lines.size() - 1);
        else
            lines.last() = lines.last().trimmed();
    }

    testAssert(lines.size() >= 2, "Invalid response");
    std::cout << "[i]\tResponse header is: " << lines.first().raw() << std::endl;
    std::cout << "[i]\tContent is: " << lines.last().raw() << std::endl;

    testAssert(lines.first() == "HTTP/1.1 200 OK"_m, "Request failed");
    testAssert(lines.last() == "it works"_m, "Request has invalid content");
    return true;
}

TEST
{
    static StackIntegrityChecker sic;

    m::URL u1;
    m::URL u2;

    testAssert(u1.parse("https://some.url:1234/doh"_m) == m::kUPE_NoError, "Failed to parse URL 1");
    testAssert(u2.parse("htTp://some.url/dah"_m) == m::kUPE_NoError, "Failed to parse URL 2");

    testAssert(u1.protocol() == "https"_m && u1.host() == "some.url"_m && u1.port() == 1234 && u1.location() == "/doh"_m, "Wrong URL 1");
    testAssert(u2.protocol() == "http"_m && u2.host() == "some.url"_m && u2.port() == 80 && u2.location() == "/dah"_m, "Wrong URL 2");

    testAssert(m::URL::encode("some test"_m) == "some%20test"_m, "URL Encoding is wrong 1");
    testAssert(m::URL::encode("some*test"_m) == "some%2Atest"_m, "URL Encoding is wrong 2");

    m::String dst;
    testAssert(m::URL::decode("some%20test"_m, dst), "URL Decoding failed 1");
    testAssert(dst == "some test"_m, "URL Decoding is wrong 1");
    testAssert(m::URL::decode("some%2Atest"_m, dst), "URL Decoding failed 2");
    testAssert(dst == "some*test"_m, "URL Decoding is wrong 2");
    return true;
}

TEST
{
    static StackIntegrityChecker sic;

    m::HTTPRequest req("http://montoyo.net/ShareX/test"_m);
    testAssert(req.perform(), "Could not perform HTTP request");
    //testAssert(req.receiveResponse(), "Could not receive HTTP response"); //req.perform() already does the job
    testAssert(req.responseCode() == 200, "Expected response code to be 200");
    testAssert(req.status() == "OK"_m, "Expected status to be OK");

    std::cout << "[i]\tContent length: " << req.contentLength() << std::endl;
    std::cout << "[i]\tDate: \"" << req.responseHeader("Date"_m).raw() << '\"' << std::endl;

    m::SSharedPtr<m::InputStream> his(req.inputStream<m::RefCounter>());
    m::StringOStream sos;
    testAssert(m::IO::transfer(&sos, his.ptr(), 256), "could not transfer http data to string");
    testAssert(sos.data().trimmed() == "it works"_m, "data does not match");
    return true;
}

TEST
{
    static StackIntegrityChecker sic;
    
    m::String content("text="_m);
    content += m::URL::encode("hello from MGPCL"_m);
    content += "&count=4";

    m::HTTPRequest req("http://montoyo.net/ShareX/test_post.php"_m);
    req.setRequestType(m::kHRT_Post);
    req.setDoesOutput(true);
    req.setContentLength(content.length());

    testAssert(req.perform(), "Could not perform HTTP POST request");

    {
        m::StringIStream input(content);
        m::SSharedPtr<m::OutputStream> output(req.outputStream<m::RefCounter>());
        testAssert(m::IO::transfer(output.ptr(), &input, 256), "could send HTTP POST data");
        testAssert(req.receiveResponse(), "could not receive HTTP POST response");
        testAssert(req.responseCode() == 200, "Expected response code to be 200");
        testAssert(req.status() == "OK"_m, "Expected status to be OK");
    }

    {
        m::SSharedPtr<m::InputStream> his(req.inputStream<m::RefCounter>());
        m::StringOStream sos;
        testAssert(m::IO::transfer(&sos, his.ptr(), 256), "could not transfer HTTP POST result to string");
        testAssert(sos.data().trimmed() == "hello from MGPCLhello from MGPCLhello from MGPCLhello from MGPCL"_m, "HTTP POST result does not match");
    }

    return true;
}

TEST
{
    static StackIntegrityChecker sic;

    m::HTTPRequest req("http://montoyo.net/ShareX/Steam_2016-09-06_20-25-29.png"_m);
    testAssert(req.perform(), "Could not perform HTTP request");
    //testAssert(req.receiveResponse(), "Could not receive HTTP response"); //req.perform() already does the job
    testAssert(req.responseCode() == 200, "Expected response code to be 200");
    testAssert(req.status() == "OK"_m, "Expected status to be OK");

    std::cout << "[i]\tContent length: " << req.contentLength() << std::endl;
    std::cout << "[i]\tDate: \"" << req.responseHeader("Date"_m).raw() << '\"' << std::endl;

    m::SSharedPtr<m::InputStream> his(req.inputStream<m::RefCounter>());
    m::FileOutputStream fos;
    testAssert(fos.open("test.png"_m, m::FileOutputStream::kOM_Truncate), "could not open output file");
    testAssert(m::IO::transfer(&fos, his.ptr(), 8192), "could not transfer http data to string");
    return true;
}

TEST
{
    static StackIntegrityChecker sic;
    m::HTTPCookieJar *jar = new m::HTTPCookieJar;

    {
        m::HTTPRequest req("http://montoyo.net/mgpcl/t1.php"_m);
        req.setCookieJar(jar);
        testAssert(req.perform(), "could not perform http request");
        testAssert(req.responseCode() == 200, "response code is not 200");
        testAssert(req.status() == "OK"_m, "status is not OK");
    }

    for(m::HTTPCookieJar::Pair &p : *jar)
        std::cout << "[i]\tGot cookie \"" << p.key.raw() << "\" with value: \"" << p.value.value().raw() << "\"" << std::endl;

    {
        m::HTTPRequest req("http://montoyo.net/mgpcl/t2.php"_m);
        req.setCookieJar(jar);
        testAssert(req.perform(), "could not perform http request");
        testAssert(req.responseCode() == 200, "response code is not 200");
        testAssert(req.status() == "OK"_m, "status is not OK");
    }

    delete jar;

    {
        m::HTTPRequest req("http://montoyo.net/mgpcl/t3.php"_m);
        req.setFollowsLocation(true);
        testAssert(req.perform(), "could not perform http request");
        testAssert(req.responseCode() == 200, "response code is not 200");
        testAssert(req.status() == "OK"_m, "status is not OK");
        testAssert(req.url().location() == "/mgpcl/t5.php"_m, "redirection didn't work?");
    }
    return true;
}

TEST
{
    static StackIntegrityChecker sic;
    m::HTTPRequest req("http://montoyo.net/ShareX/test"_m);
    req.setKeepAlive(true);

    for(int i = 0; i < 10; i++) {
        testAssert(req.perform(), "Could not perform HTTP request");
        //testAssert(req.receiveResponse(), "Could not receive HTTP response"); //req.perform() already does the job
        testAssert(req.responseCode() == 200, "Expected response code to be 200");
        testAssert(req.status() == "OK"_m, "Expected status to be OK");

        m::SSharedPtr<m::InputStream> his(req.inputStream<m::RefCounter>());
        m::StringOStream sos;
        testAssert(m::IO::transfer(&sos, his.ptr(), 256), "could not transfer http data to string");
        testAssert(sos.data().trimmed() == "it works"_m, "data does not match");
    }

    return true;
}

class ClSvTest : public m::SlotCapable
{
public:
    ClSvTest()
    {
        status = true;
        cnt = 0;
        numPings = 0;
        numPongs.set(0);
    }
    
    bool onClientConnected(m::TCPServerClient *cli)
    {
        std::cout << "[i]\t=> Client \"" << cli->address().toString().raw() << "\" connected!" << std::endl;
        cnt++;
        return false;
    }

    bool onClientDisconnected(m::TCPServerClient *cli, bool gracefull)
    {
        if(!gracefull)
            status = false;

        std::cout << "[i]\t<= Client \"" << cli->address().toString().raw() << "\" disconnected." << std::endl;
        cnt--;
        return false;
    }

    bool onClientPacket(m::TCPServerClient *cli)
    {
        m::PacketReader in(cli->nextPacket());
        m::String ping;
        in >> ping;

        if(ping == "ping"_m) {
            m::Packet out(sizeof(uint64_t) + 4);
            out << "pong"_m;

            cli->send(out.finalize());
            numPings++;
        }

        return false;
    }

    bool onServerPacket(m::TCPClient *cli)
    {
        m::PacketReader in(cli->nextPacket());
        m::String pong;
        in >> pong;

        if(pong == "pong"_m)
            numPongs.increment();

        return false;
    }

    bool status;
    int cnt;
    int numPings;
    m::Atomic numPongs;
};

TEST
{
    static StackIntegrityChecker sic;
    m::TCPServer sv;
    m::TCPClient cl[4];
    m::IPv4Address localhost(127, 0, 0, 1, 15253);

    ClSvTest test;
    sv.onClientConnected.connect(&test, &ClSvTest::onClientConnected);
    sv.onClientDisconnected.connect(&test, &ClSvTest::onClientDisconnected);
    sv.onPacketAvailable.connect(&test, &ClSvTest::onClientPacket);

    for(int i = 0; i < 4; i++)
        cl[i].onPacketAvailable.connect(&test, &ClSvTest::onServerPacket);

    testAssert(sv.listen(localhost.port()), "could not start server!");
    for(int i = 0; i < 4; i++)
        testAssert(cl[i].connect(localhost) == m::kSCE_NoError, "couldn't connect client!");

    m::time::sleepMs(100);
    testAssert(test.cnt == 4, "invalid client count 1");

    m::Packet pkt(sizeof(uint16_t) + 4);
    pkt << "ping"_m;

    m::FPacket fpkt(pkt.finalize());
    for(int i = 0; i < 4; i++)
        cl[i].send(fpkt.duplicate());

    fpkt.destroy();

    double start = m::time::getTimeMs();
    do {
        testAssert(m::time::getTimeMs() - start < 2500, "still didn't receive pongs!");
        m::time::sleepMs(10);
    } while(test.numPongs.get() < 4);

    for(int i = 0; i < 4; i++)
        cl[i].stop();

    m::time::sleepMs(10);
    sv.stop();

    testAssert(test.cnt == 0, "invalid client count 2");
    testAssert(test.numPings == 4, "invalid ping count");
    testAssert(test.status, "one or more client errored on server side!");
    return true;
}

#ifndef MGPCL_NO_SSL

TEST
{
    volatile StackIntegrityChecker sic;
    m::HTTPRequest req("https://montoyo.net/ShareX/test_ssl.php"_m);
    testAssert(req.perform(), req.sslErrorString().raw());
    testAssert(req.responseCode() == 200, "Expected HTTPS response code to be 200");
    testAssert(req.status() == "OK"_m, "Expected HTTPS status to be OK");

    std::cout << "[i]\tSSL Content length: " << req.contentLength() << std::endl;
    std::cout << "[i]\tSSL Date: \"" << req.responseHeader("Date"_m).raw() << '\"' << std::endl;

    m::SSharedPtr<m::InputStream> his(req.inputStream<m::RefCounter>());
    m::SSharedPtr<m::StringOStream> sos(new m::StringOStream);
    testAssert(m::IO::transfer(sos.ptr(), his.ptr(), 256), "could not transfer HTTPS data to string");
    testAssert(sos->data().trimmed() == "it works"_m, "HTTPS data does not match");
    return true;
}

TEST
{
    static StackIntegrityChecker sic;
    m::HTTPRequest req("https://montoyo.net/ShareX/test"_m);
    req.setKeepAlive(true);

    for(int i = 0; i < 10; i++) {
        testAssert(req.perform(), "Could not perform HTTP request");
        //testAssert(req.receiveResponse(), "Could not receive HTTP response"); //req.perform() already does the job
        testAssert(req.responseCode() == 200, "Expected response code to be 200");
        testAssert(req.status() == "OK"_m, "Expected status to be OK");

        m::SSharedPtr<m::InputStream> his(req.inputStream<m::RefCounter>());
        m::StringOStream sos;
        testAssert(m::IO::transfer(&sos, his.ptr(), 256), "could not transfer http data to string");
        testAssert(sos.data().trimmed() == "it works"_m, "data does not match");
    }

    return true;
}

#endif
