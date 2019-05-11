#define M_HTST_C
#include "HTTPServerTest.h"
#include <mgpcl/HTTPServer.h>
#include <mgpcl/Time.h>

static m::Atomic g_running(1);

class ShutdownHandler : public m::HTTPRequestHandler
{
public:
    void beginRequest(m::HTTPServerRequest *req) override
    {
    }

    void receiveData(m::HTTPServerRequest *req, uint8_t *data, int sz) override
    {
    }

    void processRequest(m::HTTPServerRequest *req) override
    {
        g_running.set(0);

        if(req->method() != m::kHRT_Head)
            req->setResponseLength(0);
    }

    int sendData(m::HTTPServerRequest *req, uint8_t *dst, int dstSz) override
    {
        return 0;
    }

    void finishRequest(m::HTTPServerRequest *req, bool success) override
    {
    }
};

class EchoHandler : public m::SimpleHTTPRequestHandler
{
public:
    void processRequest(m::HTTPServerRequest *req) override
    {
        m::SimpleHTTPRequestHandler::SimpleUserdata *ud = static_cast<m::SimpleHTTPRequestHandler::SimpleUserdata*>(req->userdata());
        ud->setResponse(ud->requestContent() + "\n" + req->wildcard(0) + "\ntest val=" + req->urlParam("test"));

        req->setResponse(200, "OK");
        req->setResponseHeader("Content-Type", "text/plain");
        m::SimpleHTTPRequestHandler::processRequest(req);
    }
};

void mTestHTTPServer()
{
    m::time::initTime();
    m::inet::initialize();
    m::inet::initSSL();

    m::HTTPServer server;
    server.bindHandler("/", new m::StaticHTTPRequestHandler("<!DOCTYPE html><html lang=\"en\"><head><title>Hi</title></head><body><h1>It Works!</h1></body></html>"));
    server.bindHandler("/form/*/test", new m::StaticHTTPRequestHandler("<!DOCTYPE html><html lang=\"en\"><head><title>Form Test</title></head><body><form action=\"echo\" method=\"POST\"><input type=\"submit\" name=\"test\" /></form></body></html>"));
    server.bindHandler("/form/*/echo", new EchoHandler);
    server.bindHandler("/shutdown", new ShutdownHandler);

#ifndef MGPCL_NO_SSL
    server.enableSSL("certificate.pem", "key.pem");
#endif

    server.start(m::IPv4Address(127, 0, 0, 1, 1234), 4);

    while(g_running.get() != 0)
        m::time::sleepMs(100);

    server.stop();
}
