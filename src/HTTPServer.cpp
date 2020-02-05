/* Copyright (C) 2020 BARBOTIN Nicolas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "mgpcl/HTTPServer.h"
#include "mgpcl/Time.h"
#include "mgpcl/Math.h"
#include "mgpcl/Date.h"
#include <iostream>

//#define M_TRACE_HTTPSERVER

#if defined(_DEBUG) && defined(M_TRACE_HTTPSERVER)
#define M_TRACE(msg) std::cout << msg << std::endl;
#else
#define M_TRACE(msg)
#endif

static const m::String g_404data("<!DOCTYPE html><html lang=\"en\"><head><title>Error 404</title></head><body><h1>404 Not Found</h1></body></html>"_m);

static void splitPathname(const m::String &str, m::List<m::String> &dst)
{
    int last = 1;

    for(int i = 1; i < str.length(); i++) {
        if(str[i] == '/') {
            if(i > last)
                dst.add(str.substr(last, i));

            last = i + 1;
        }
    }

    if(last < str.length())
        dst.add(str.substr(last));
}

m::HTTPServer::HTTPServer() : m_running(1), m_inactivityTimeout(5000), m_root(nullptr)
{
    StaticHTTPRequestHandler *shrh = new StaticHTTPRequestHandler(g_404data);
    shrh->setResponseCode(404, "Not Found"_m);
    m_404handler = shrh;

    m_server.setConnectionTimeout(0); //Non-blocking accept
    m_server.setReadAndWriteTimeouts(0);
}

m::HTTPServer::~HTTPServer()
{
    stop(false);

    for(int i = 0; i < m_threadPool.count(); i++)
        delete worker(i);

    if(m_root != nullptr)
        delete m_root;
}

#ifndef MGPCL_NO_SSL

bool m::HTTPServer::enableSSL(const String &certFile, const String &keyFile)
{
    if(!m_sslCtx.initialize(kSCM_v23Server))
        return false;

    m_sslCtx.enableAutoECDH(true);
    return m_sslCtx.useCertificateFile(certFile) && m_sslCtx.usePrivateKeyFile(keyFile);
}

bool m::HTTPServer::enableSSL(const SSLContext &ctx)
{
    if(!ctx)
        return false;

    m_sslCtx = ctx;
    return true;
}

#endif

bool m::HTTPServer::start(const IPv4Address &listenAddr, int numThreads)
{
    //Start TCP server
    if(!m_server.initialize())
        return false;

    if(!m_server.bind(listenAddr))
        return false;

    if(!m_server.listen())
        return false;

    //Configure threads
    m_threadPool.setCount(numThreads);
    m_threadPool.setName("HSW-"_m);
    m_threadPool.setCallback(Worker::run);

    for(int i = 0; i < numThreads; i++)
        m_threadPool.setUserdata(i, new Worker(this));

    m_threadPool.start();
    return true;
}

void m::HTTPServer::stop(bool graceful)
{
    if(m_running.get() != 0) {
        m_running.set(0);
        
        if(graceful) {
            //Not so graceful... but ok

            for(int i = 0; i < m_threadPool.count(); i++)
                worker(i)->stopClients();
        }

        m_threadPool.joinAll();
        m_server.close();
    }
}

void m::HTTPServer::set404Handler(HTTPRequestHandler *h)
{
    if(m_404handler == h)
        return;

    delete m_404handler;

    if(h == nullptr) {
        StaticHTTPRequestHandler *shrh = new StaticHTTPRequestHandler(g_404data);
        shrh->setResponseCode(404, "Not Found"_m);

        m_404handler = shrh;
    } else
        m_404handler = h;
}

void m::HTTPServer::bindHandler(const String &path, HTTPRequestHandler *h)
{
    mAssert(path.startsWith("/"_m), "path should start with a slash");

    List<String> components;
    String fixedPath(http::smartEncodePathname(path));
    splitPathname(fixedPath, components);

    if(m_root == nullptr)
        m_root = new Node;

    Node *n = m_root;
    for(const String &component: components) {
        bool isMatchAll = component == "*"_m;
        Node *child = isMatchAll ? n->m_fallbackChild : n->child(component);

        if(child == nullptr) {
            child = new Node(component);

            if(isMatchAll)
                n->m_fallbackChild = child;
            else
                n->m_children.add(child);
        }

        n = child;
    }

    if(h != n->m_handler) {
        if(n->m_handler != nullptr)
            delete n->m_handler;

        n->m_handler = h;
    }
}

void m::HTTPServer::dispatchClient(const IPv4Address &addr, TCPSocket &&cli)
{
    worker(m_dispatcher.increment() % m_threadPool.count())->addClient(addr, std::move(cli));
}

m::HTTPServer::Worker::~Worker()
{
    for(Client *cli : m_clients)
        delete cli;
}

void m::HTTPServer::Worker::run()
{
    while(m_parent->m_running.get()) {
        uint32_t now = time::getTimeMsUInt();
        TCPSocketSet rdSet;
        TCPSocketSet wrSet;

        m_selectedClients.cleanup();
        rdSet.add(m_parent->m_server);

        m_clientLock.lock();
        for(int i = ~m_clients - 1; i >= 0; i--) {
            Client *cli = m_clients[i];

            if(cli->m_shouldRemove || now - cli->m_time >= m_parent->m_inactivityTimeout) {
                m_clients.remove(i);
                delete cli;
            } else if(cli->shouldRead()) {
                rdSet.add(*cli->m_socket);
                m_selectedClients.add(cli);
            } else if(cli->shouldWrite()) {
                wrSet.add(*cli->m_socket);
                m_selectedClients.add(cli);
            }
        }
        m_clientLock.unlock();

        int cnt = TCPSocketSet::waitForSets(&rdSet, &wrSet, nullptr, 10);
        if(cnt < 0)
            return; //FIXME: What to do ?

        if(rdSet.isSet(m_parent->m_server)) {
            IPv4Address addr;
            TCPSocket sock(m_parent->m_server.accept(addr));

            if(sock.isValid())
                m_parent->dispatchClient(addr, std::move(sock));

            cnt--;
        }

        if(cnt > 0) {
            for(Client *cli: m_selectedClients) {
                if(rdSet.isSet(*cli->m_socket) || wrSet.isSet(*cli->m_socket)) {
                    cli->comReady();

                    if(--cnt <= 0)
                        break;
                }
            }
        }
    }

    for(Client *cli: m_clients)
        delete cli;

    m_clients.clear();
    m_selectedClients.clear();
}

void m::HTTPServer::Worker::addClient(const IPv4Address &addr, TCPSocket &&cli)
{
    Client *hsc;

#ifdef MGPCL_NO_SSL
    hsc = new Client(this, addr, new TCPSocket(std::move(cli)), false);
#else
    if(m_parent->m_sslCtx.isValid()) {
        SSLSocket *ssls = new SSLSocket;
        SSLAcceptError err = ssls->initializeAndAccept(m_parent->m_sslCtx, cli);

        if(err == kSAE_NoError) {
            hsc = new Client(this, addr, ssls, true);
            M_TRACE("handshake on accept ok");
        } else if(err == kSAE_SSLHandshakeTimeout)
            hsc = new Client(this, addr, ssls, ssls->lastWantedOperation());
        else {
            delete ssls;
            return;
        }
    } else
        hsc = new Client(this, addr, new TCPSocket(std::move(cli)), false);
#endif

    m_clientLock.lock();
    m_clients.add(hsc);
    m_clientLock.unlock();
    M_TRACE("worker accepted client");
}

void m::HTTPServer::Worker::stopClients()
{
    for(Client *c: m_clients)
        c->stopClient();
}

void m::HTTPServer::Worker::run(void *data)
{
    static_cast<Worker*>(data)->run();
}

#ifdef MGPCL_NO_SSL
m::HTTPServer::Client::Client(Worker *p, const IPv4Address &addr, TCPSocket *sock, bool ssl) : m_parent(p), m_addr(addr),
                                                                                               m_socket(sock), m_phase(kHRP_Read),
                                                                                               m_shouldRemove(false), m_lineLength(0), m_readPhase(kHRRP_QueryLine),
                                                                                               m_writingHeaders(true), m_handler(nullptr), m_remDataLen(0),
                                                                                               m_sentLinePos(0)
{
    m_time = time::getTimeMsUInt();
    m_req = new HTTPServerRequest;
}
#else
m::HTTPServer::Client::Client(Worker *p, const IPv4Address &addr, TCPSocket *sock, bool ssl) : m_parent(p), m_isSSL(ssl), m_addr(addr),
                                                                                               m_socket(sock), m_sslOP(kSWO_WantRead), m_phase(kHRP_Read),
                                                                                               m_shouldRemove(false), m_lineLength(0), m_readPhase(kHRRP_QueryLine),
                                                                                               m_writingHeaders(true), m_handler(nullptr), m_remDataLen(0),
                                                                                               m_sentLinePos(0)
{
    m_time = time::getTimeMsUInt();
    m_req = new HTTPServerRequest;
}

m::HTTPServer::Client::Client(Worker *p, const IPv4Address &addr, SSLSocket *sock, SSLWantedOperation handshakeOp) : m_parent(p), m_isSSL(true), m_addr(addr),
                                                                                                                     m_socket(sock), m_sslOP(handshakeOp), m_phase(kHRP_Handshake),
                                                                                                                     m_shouldRemove(false), m_lineLength(0), m_readPhase(kHRRP_QueryLine),
                                                                                                                     m_writingHeaders(true), m_req(nullptr), m_handler(nullptr),
                                                                                                                     m_remDataLen(0), m_sentLinePos(0)
{
    m_time = time::getTimeMsUInt();
}
#endif

m::HTTPServer::Client::~Client()
{
    if(m_req != nullptr) {
        if(m_handler != nullptr)
            m_handler->finishRequest(m_req, false);

        delete m_req;
    }

    stopClient();
    delete m_socket;
    M_TRACE("destroyed client");
}

void m::HTTPServer::Client::comReady()
{
    if(m_phase == kHRP_Handshake) {
#ifndef MGPCL_NO_SSL
        SSLAcceptError sae = static_cast<SSLSocket*>(m_socket)->resumeAcceptHandshake();

        if(sae == kSAE_NoError) {
            m_req = new HTTPServerRequest;
            m_phase = kHRP_Read;
            m_sslOP = kSWO_WantRead;
            m_time = time::getTimeMsUInt();
            M_TRACE("client handshake successful");
        } else if(sae != kSAE_SSLHandshakeTimeout)
            removeDueToError("SSL handshake error");
#endif
    } else if(m_phase == kHRP_Read) {
        int diff = M_HTTP_SERVER_RBUF_SZ - m_lineLength;
        int rd = m_socket->receive(m_recvBuf, diff);

        if(rd < 0) {
            if(m_socket->lastError() == inet::kSE_NoError) {
#ifndef MGPCL_NO_SSL
                if(m_isSSL)
                    m_sslOP = static_cast<SSLSocket*>(m_socket)->lastWantedOperation();
#endif
            } else
                removeDueToError("read failure");
        } else if(rd == 0)
            removeDueToError("client connection closed unexpectedly");
        else if(m_readPhase == kHRRP_Content) {
            m_time = time::getTimeMsUInt();
            m_handler->receiveData(m_req, m_recvBuf, rd);
            m_remDataLen -= static_cast<uint64_t>(rd);

            if(m_remDataLen <= 0)
                startResponse();
        } else {
            char *buf = reinterpret_cast<char*>(m_recvBuf);
            int startAt = m::math::maximum(m_lineLength - 1, 0);
            int lineEnd;

            m_time = time::getTimeMsUInt();
            m_lineLength += rd;

            do {
                lineEnd = -1;

                for(int i = startAt; i < m_lineLength - 1; i++) {
                    if(buf[i] == '\r' && buf[i + 1] == '\n') {
                        lineEnd = i;
                        break;
                    }
                }

                startAt = 0;

                if(lineEnd < 0) {
                    if(m_lineLength >= M_HTTP_SERVER_RBUF_SZ)
                        removeDueToError("client reached max header line length");
                } else {
                    if(m_readPhase == kHRRP_QueryLine) {
                        if(lineEnd < 14)
                            removeDueToError("request line is malformed");
                        else {
                            m_responseBuffer.append(buf, lineEnd);
                            m::String upperLine(m_responseBuffer.upper());

                            if(!upperLine.endsWith(" HTTP/1.1"_m) && !upperLine.endsWith(" HTTP/1.0"_m))
                                removeDueToError("invalid protocol");
                            else {
                                HTTPRequestType method;
                                int methodLen = -1;

                                if(upperLine.startsWith("GET "_m)) {
                                    method = kHRT_Get;
                                    methodLen = 4;
                                } else if(upperLine.startsWith("POST "_m)) {
                                    method = kHRT_Post;
                                    methodLen = 5;
                                } else if(upperLine.startsWith("HEAD "_m)) {
                                    method = kHRT_Head;
                                    methodLen = 5;
                                } else if(upperLine.startsWith("PUT "_m)) {
                                    method = kHRT_Put;
                                    methodLen = 4;
                                } else if(upperLine.startsWith("DELETE "_m)) {
                                    method = kHRT_Delete;
                                    methodLen = 7;
                                }

                                if(methodLen < 0)
                                    removeDueToError("unsupported request method");
                                else {
                                    m_req->m_method = method;
                                    m_req->m_pathname = String(buf + methodLen, lineEnd - methodLen - 9);
                                    m_req->m_pathname = m_req->m_pathname.trimmed();

                                    if(m_req->m_pathname.startsWith("/", 1)) {
                                        m_req->m_pathname = http::smartEncodePathname(m_req->m_pathname);
                                        m_readPhase = kHRRP_Headers;

                                        M_TRACE("sucessfuly parsed first line");
                                    } else
                                        removeDueToError("invalid request path");
                                }
                            }
                        }
                    } else if(m_readPhase == kHRRP_Headers) {
                        int dpPos = -1;
                        for(int i = 0; i < lineEnd; i++) {
                            if(buf[i] == ':') {
                                dpPos = i;
                                break;
                            }
                        }

                        if(lineEnd == 0) {
                            m_readPhase = kHRRP_Content;
                            onHeadersReceived();

                            if(m_remDataLen <= 0)
                                startResponse();
                            else if(m_lineLength > 2) {
                                m_handler->receiveData(m_req, m_recvBuf + 2, m_lineLength - 2);
                                m_remDataLen -= static_cast<uint64_t>(m_lineLength - 2);

                                if(m_remDataLen <= 0)
                                    startResponse();
                            }
                        } else if(dpPos <= 0)
                            removeDueToError("invalid header line: missing colons");
                        else {
                            String key(buf, dpPos);
                            String value(buf + dpPos + 1, lineEnd - dpPos - 1);

                            m_req->m_queryHeaders[key.trimmed()] = value.trimmed();
                            M_TRACE("received header");
                        }
                    }

                    int newLen = m_lineLength - lineEnd - 2;
                    m::mem::move(m_recvBuf, m_recvBuf + lineEnd + 2, newLen);
                    m_lineLength = newLen;
                }
            } while(lineEnd >= 0 && m_phase == kHRP_Read && (m_readPhase == kHRRP_QueryLine || m_readPhase == kHRRP_Headers));
        }
    } else if(m_phase == kHRP_Write) {
        int written;

        if(m_writingHeaders) {
            int delta = m_responseBuffer.length() - static_cast<int>(m_remDataLen);
            written = m_socket->send(reinterpret_cast<const uint8_t*>(m_responseBuffer.raw()) + delta, static_cast<int>(m_remDataLen));
        } else {
            if(m_lineLength <= 0) {
                m_lineLength = m_handler->sendData(m_req, m_recvBuf, M_HTTP_SERVER_RBUF_SZ);
                m_sentLinePos = 0;
            }

            written = m_socket->send(m_recvBuf + m_sentLinePos, m_lineLength);

            if(written > 0) {
                m_sentLinePos += written;
                m_lineLength -= written;
            }
        }

        if(written < 0) {
            if(m_socket->lastError() == inet::kSE_NoError) {
#ifndef MGPCL_NO_SSL
                if(m_isSSL)
                    m_sslOP = static_cast<SSLSocket*>(m_socket)->lastWantedOperation();
#endif
            } else
                removeDueToError("write failure");
        } else if(written == 0)
            removeDueToError("client connection closed unexpectedly");
        else {
            m_time = time::getTimeMsUInt();
            m_remDataLen -= static_cast<uint64_t>(written);

            if(m_remDataLen <= 0) {
                if(m_writingHeaders) {
                    m_writingHeaders = false;
                    m_responseBuffer.clear();

                    m_remDataLen = m_req->m_responseLength;
                    m_lineLength = 0;

                    if(m_remDataLen <= 0)
                        finish();
                } else
                    finish();
            }
        }
    } else if(m_phase == kHRP_Shutdown) {
#ifndef MGPCL_NO_SSL
        if(static_cast<SSLSocket*>(m_socket)->shutdown()) {
            m_shouldRemove = true;
            M_TRACE("SSL shutdown complete");
        } else if(m_socket->lastError() != inet::kSE_NoError)
            removeDueToError("SSL shutdown error");
#endif
    }
}

void m::HTTPServer::Client::removeDueToError(const char *err)
{
    //TODO: print error properly (= not in stderr)

#if defined(_DEBUG) && defined(M_TRACE_HTTPSERVER)
    std::cerr << "HTTPServer: " << err << std::endl;
#endif

    m_shouldRemove = true;
}

void m::HTTPServer::Client::onHeadersReceived()
{
    M_TRACE("all headers received");
    int paramsBegin = m_req->m_pathname.indexOf('?');

    if(paramsBegin >= 0) {
        String paramsPart(m_req->m_pathname.substr(paramsBegin + 1));
        m_req->m_pathname = m_req->m_pathname.substr(0, paramsBegin);

        List<String> paramsAssign;
        paramsPart.splitOn('&', paramsAssign);

        for(const String &p: paramsAssign) {
            int eqPos = p.indexOf('=');
            String key(p.substr(0, eqPos)); //If not found then -1 which means end, so it's ok
            key = http::decodeURIComponent(key);

            if(eqPos >= 0)
                m_req->m_queryParams[key] = http::decodeURIComponent(p.substr(eqPos + 1));
            else
                m_req->m_queryParams[key] = key;
        }
    }

    List<String> components;
    splitPathname(m_req->m_pathname, components);

    Node *n = m_parent->m_parent->m_root;
    for(const String &component: components) {
        Node *child = n->child(component);

        if(child == nullptr) {
            n = n->m_fallbackChild;
            if(n == nullptr)
                break;

            m_req->m_pathWildcards.add(component);
        } else
            n = child;
    }

    const String clKey("Content-Length"_m);
    if(m_req->m_queryHeaders.hasKey(clKey)) {
        m_remDataLen = static_cast<uint64_t>(m_req->m_queryHeaders[clKey].toUInteger());
        m_req->m_queryLength = m_remDataLen;
    }

    m_handler = (n == nullptr || n->m_handler == nullptr) ? m_parent->m_parent->m_404handler : n->m_handler;
    m_handler->beginRequest(m_req);

#ifndef MGPCL_NO_SSL
    m_sslOP = kSWO_WantWrite;
#endif
}

void m::HTTPServer::Client::startResponse()
{
    M_TRACE("beginning response");

    m_phase = kHRP_Write;

    double took = m::time::getTimeMs();
    m_handler->processRequest(m_req);
    took = m::time::getTimeMs() - took;

    if(!m_parent->m_parent->m_accessLog.isNull()) {
        m::String &buf = m_parent->m_accessBuf;

        buf.cleanup();
        buf += m_addr.toString(false);
        buf += " - - ["_m;
        buf += Date::now().format("%D/%n/%y:%H:%M:%S"_m);
        buf += "] \""_m;
        buf += m_responseBuffer;
        buf += "\" "_m;
        buf += m::String::fromInteger(m_req->m_responseCode);
        buf.append(' ', 1);
        buf += m::String::fromUInteger(static_cast<uint32_t>(m_req->m_responseLength));
        buf.append(' ', 1);
        buf += m::String::fromDouble(took, 4);
        buf += M_OS_LINEEND;

        m_parent->m_parent->m_accessLogLock.lock();
        m_parent->m_parent->m_accessLog->write(reinterpret_cast<const uint8_t*>(buf.raw()), buf.length());
        m_parent->m_parent->m_accessLogLock.unlock();
    }

    m_responseBuffer.cleanup();
    m_responseBuffer += "HTTP/1.1 "_m;
    m_responseBuffer += String::fromInteger(m_req->m_responseCode);
    m_responseBuffer += ' ';
    m_responseBuffer += m_req->m_responseMessage;
    m_responseBuffer += "\r\n"_m;

    for(const HashMap<String, String, StringLowerHasher>::Pair &p: m_req->m_responseHeaders) {
        m_responseBuffer += p.key;
        m_responseBuffer += ": "_m;
        m_responseBuffer += p.value;
        m_responseBuffer += "\r\n"_m;
    }

    m_responseBuffer += "\r\n"_m;
    m_remDataLen = m_responseBuffer.length();
}

void m::HTTPServer::Client::finish()
{
    M_TRACE("query finished");

    m_handler->finishRequest(m_req, true);
    delete m_req;
    m_req = nullptr;

#ifdef MGPCL_NO_SSL
    m_shouldRemove = true;
#else
    if(m_isSSL)
        m_phase = kHRP_Shutdown;
    else
        m_shouldRemove = true;
#endif
}

void m::HTTPServer::Client::stopClient()
{
#ifdef MGPCL_NO_SSL
    m_socket->close();
#else
    if(m_isSSL)
        static_cast<SSLSocket*>(m_socket)->close(false);
    else
        m_socket->close();
#endif

    m_shouldRemove = true;
}

m::HTTPServer::Node::~Node()
{
    if(m_handler != nullptr)
        delete m_handler;

    for(Node *c: m_children)
        delete c;
}

m::HTTPServer::Node *m::HTTPServer::Node::child(const String &str)
{
    for(Node *n: m_children) {
        if(n->m_match == str)
            return n;
    }

    return nullptr;
}

void m::SimpleHTTPRequestHandler::beginRequest(HTTPServerRequest *req)
{
    req->setUserdata(createUserdata());
}

void m::SimpleHTTPRequestHandler::receiveData(HTTPServerRequest *req, uint8_t *data, int sz)
{
    if(req->requestLength() < m_maxRecvLen)
        static_cast<SimpleUserdata*>(req->userdata())->receiveRequest(data, sz);
}

void m::SimpleHTTPRequestHandler::processRequest(HTTPServerRequest *req)
{
    if(req->method() != kHRT_Head)
        req->setResponseLength(static_cast<uint64_t>(static_cast<SimpleUserdata*>(req->userdata())->responseContent().length()));
}

int m::SimpleHTTPRequestHandler::sendData(HTTPServerRequest *req, uint8_t *dst, int dstSz)
{
    return static_cast<SimpleUserdata*>(req->userdata())->sendResponse(dst, dstSz);
}

void m::SimpleHTTPRequestHandler::finishRequest(HTTPServerRequest *req, bool success)
{
    if(req->userdata() != nullptr)
        delete static_cast<SimpleUserdata*>(req->userdata());
}

m::SimpleHTTPRequestHandler::SimpleUserdata *m::SimpleHTTPRequestHandler::createUserdata()
{
    return new SimpleUserdata;
}

void m::SimpleHTTPRequestHandler::SimpleUserdata::receiveRequest(const uint8_t *data, int len)
{
    m_recv.append(reinterpret_cast<const char*>(data), len);
}

int m::SimpleHTTPRequestHandler::SimpleUserdata::sendResponse(uint8_t *dst, int sz)
{
    int toWrite = m::math::minimum(sz, m_send.length() - m_pos);
    m::mem::copy(dst, m_send.raw() + m_pos, toWrite);
    m_pos += toWrite;

    return toWrite;
}

void m::StaticHTTPRequestHandler::beginRequest(HTTPServerRequest *req)
{
    req->setUserdata(new int(0));
}

void m::StaticHTTPRequestHandler::receiveData(HTTPServerRequest *req, uint8_t *data, int sz)
{
}

void m::StaticHTTPRequestHandler::processRequest(HTTPServerRequest *req)
{
    req->setResponse(m_status, m_message);

    if(req->method() != kHRT_Head) {
        req->setResponseHeader("Content-Type"_m, m_contentType);
        req->setResponseLength(static_cast<uint64_t>(m_data.length()));
    }
}

int m::StaticHTTPRequestHandler::sendData(HTTPServerRequest *req, uint8_t *dst, int sz)
{
    int &pos = *static_cast<int*>(req->userdata());
    int toWrite = m::math::minimum(sz, m_data.length() - pos);
    m::mem::copy(dst, m_data.raw() + pos, toWrite);
    pos += toWrite;

    return toWrite;
}

void m::StaticHTTPRequestHandler::finishRequest(HTTPServerRequest *req, bool success)
{
    if(req->userdata() != nullptr)
        delete static_cast<int*>(req->userdata());
}
