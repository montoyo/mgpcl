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

#include "mgpcl/HTTPRequest.h"

void m::HTTPRequest::setURL(const URL &url)
{
    if(m_conn != nullptr && m_requestHdr["Connection"_m].equalsIgnoreCase("keep-alive"_m)) {
        if(!url.protocol().equalsIgnoreCase(m_url.protocol()) || !url.host().equalsIgnoreCase(m_url.host()) || url.port() != m_url.port()) {
            //Not the same server; destroy connection
            delete m_conn;
            m_conn = nullptr;
        }
    }

    m_url = url;
}

bool m::HTTPRequest::perform()
{
    const bool keepAlive = m_requestHdr["Connection"_m].equalsIgnoreCase("keep-alive"_m);

    for(uint8_t maxRedir = 0; maxRedir < 16; maxRedir++) {
        if(!keepAlive || m_conn == nullptr) {
            if(m_conn != nullptr) {
                delete m_conn;
                m_conn = nullptr;
            }

            if(!m_url.isValid() || (m_url.protocol() != "http"_m && m_url.protocol() != "https"_m))
                return false;

            IPv4Address addr;
            if(addr.resolve(m_url.host(), m_url.port()) != kRE_NoError)
                return false;

            if(m_url.protocol() == "https"_m) {
#ifndef MGPCL_NO_SSL
                if(!m_sslCtx.isValid()) {
                    m_sslCtx.initialize(kSCM_v23Client);
                    m_sslCtx.loadOSVerify();
                    m_sslCtx.setVerifyFlags(kSVF_VerifyPeer);
                    m_sslCtx.setVerifyDepth(16); //Is that a good value?
                }

                m_conn = new SSLSocket;
                if(!static_cast<SSLSocket*>(m_conn)->initialize(m_sslCtx))
                    return false;
#else
                return false;
#endif
            } else {
                m_conn = new TCPSocket;
                if(!m_conn->initialize())
                    return false;
            }

            if(m_conn->connect(addr) != kSCE_NoError)
                return false;
        }

        String request(16);
        switch(m_type) {
        case kHRT_Get:
            request += "GET"_m;
            break;

        case kHRT_Post:
            request += "POST"_m;
            break;

        case kHRT_Put:
            request += "PUT"_m;
            break;

        case kHRT_Head:
            request += "HEAD"_m;
            break;

        case kHRT_Delete:
            request += "DELETE"_m;
            break;

        case kHRT_Trace:
            request += "TRACE"_m;
            break;

        case kHRT_Connect:
            request += "CONNECT"_m;
            break;

        default:
            m_conn->close();
            return false;
        }

        request += ' ';
        if(m_url.location().isEmpty())
            request += "/ HTTP/1.1\r\n"_m;
        else {
            request += m_url.location();
            request += " HTTP/1.1\r\n"_m;
        }

        request += "Host: "_m;
        request += m_url.host();
        request += "\r\n"_m;

        if(!m_requestHdr.hasKey("User-Agent"_m))
            m_requestHdr["User-Agent"_m] = "MGPCL v" MGPCL_VERSION_STRING;

        if(m_doesOut && !m_requestHdr.hasKey("Content-Type"_m))
            m_requestHdr["Content-Type"_m] = "application/x-www-form-urlencoded"_m;

        for(HashMap<String, String, StringLowerHasher>::Pair &p : m_requestHdr) {
            request += p.key;
            request += ": "_m;
            request += p.value;
            request += "\r\n"_m;
        }

        time_t now = Date::unixTime();
        if(m_jar != nullptr) {
            //RFC6265 requires cookies to be sorted by path length but
            //really I don't give a single shit. It'd require more code
            //and I don't see how or why it'd make things faster or safer.
            //
            //Apparently some old servers required this in order to work.
            //Someone please put a bomb in these servers. Thank you,
            // ~montoyo

            bool first = true;
            String tmp(16);

            for(HTTPCookieJar::Pair &p : *m_jar) {
                if(p.value.isValid() && p.value.isValid(now) && p.value.isSuitableFor(m_url)) {
                    if(first)
                        first = false;
                    else
                        tmp += "; "_m;

                    tmp += p.key;
                    tmp += '=';
                    tmp += p.value.value();
                }
            }

            if(!first) { //Same as !tmp.isEmpty()
                request += "Cookie: "_m;
                request += tmp;
                request += "\r\n"_m;
            }
        }

        request += "\r\n"_m;

        const char *str = request.raw();
        int len = request.length();

        while(len > 0) {
            int w = m_conn->send(reinterpret_cast<const uint8_t*>(str), len);
            if(w <= 0) {
                m_conn->close();
                return false;
            }

            str += w;
            len -= w;
        }

        m_gotResponse = false;
        if(m_doesOut)
            return true;

        //If it doesn't write anything else, read response now
        if(!receiveResponse())
            return false;

        if(!m_followsLoc || !m_responseHdr.hasKey("Location"_m))
            return true; //Redirection disabled or end of redirection

        URL redir;
        if(redir.parseRelative(m_url, m_responseHdr["Location"_m]) != kUPE_NoError)
            return false; //Couldn't parse redirection URL

        //URL has changed to the new location, start over...
        setURL(redir); //Use setURL to close connection if keep-alive is enabled and server changed
    }

    //If this happens, then we reached the maximum number of redirections
    //So I guess the request failed...
    return false;
}

bool m::HTTPRequest::receiveResponse()
{
    return receiveResponse(m_requestHdr["Connection"_m].equalsIgnoreCase("keep-alive"_m));
}

bool m::HTTPRequest::receiveResponse(bool keepAlive)
{
    if(m_gotResponse)
        return true;
    
    if(!m_conn->isValid())
        return false;

    m_responseHdr.clear();

    m_lr.setSource(m_conn->inputStream<RefCounter>());
    int dbg = m_lr.next();

    if(dbg <= 0) {
        m_conn->close();
        return false;
    }

    const String &rline = m_lr.line();
    if(!rline.startsWith("HTTP/1."_m)) {
        m_conn->close();
        return false;
    }

    int s1 = rline.indexOf(' ') + 1;
    int s2 = rline.indexOf(' ', s1);
    if(s1 <= 0 || s2 <= s1) {
        m_conn->close();
        return false;
    }

    m_rcode = rline.substr(s1, s2).toInteger();
    m_status = rline.substr(s2 + 1);

    int hdrCnt = 0;
    while(hdrCnt < 256) {
        if(m_lr.next() <= 0) {
            m_conn->close();
            return false;
        }

        if(rline.isEmpty())
            break;

        int sep = rline.indexOf(':');
        if(sep < 0)
            return false;

        String name(rline.substr(0, sep).trimmed());
        if(name.equalsIgnoreCase("Set-Cookie"_m)) {
            if(m_jar != nullptr)
                m_jar->parseAndPutCookie(rline.substr(sep + 1).trimmed());
        } else
            m_responseHdr[name] = rline.substr(sep + 1).trimmed();

        hdrCnt++;
    }

    m_gotResponse = true;
    if(!keepAlive && !m_doesIn) //The server won't return any data, close stream now
        m_conn->close();

    return true;
}

int m::HTTPInputStream::read(uint8_t *dst, int sz)
{
    //TODO: Wrap chunked encoding in here

    mDebugAssert(sz >= 0, "cannot read a negative amount of bytes");
    uint32_t usz = static_cast<uint32_t>(sz);
    int ret = 0;

    if(m_pos < m_req->m_lr.remainingDataLength()) {
        uint32_t rd = m_req->m_lr.remainingDataLength() - m_pos;
        if(rd > usz)
            rd = usz;

        //Copy from line buffer
        mem::copy(dst, m_req->m_lr.remainingData(), rd);
        m_pos += rd;
        dst += rd;
        usz -= static_cast<int>(rd);
        ret += static_cast<int>(rd);
    }

    if(usz > 0) {
        if(m_hasLen && m_pos >= m_len) {
            if(!m_keepAlive)
                m_req->m_conn->close();

            return ret;
        }

        int rd = m_req->m_conn->receive(dst, static_cast<int>(usz));
        if(rd > 0) {
            m_pos += static_cast<uint32_t>(rd);
            ret += rd;
        } else if(rd < 0)
            return rd;
    }

    return ret;
}

m::HTTPInputStream::HTTPInputStream(HTTPRequest *p)
{
    m_req = p;
    m_pos = 0;

    const String key("Content-Length"_m);
    if(p->m_responseHdr.hasKey(key)) {
        int ret = p->m_responseHdr[key].toInteger();

        if(ret < 0) {
            m_hasLen = false;
            m_len = 0;
        } else {
            m_hasLen = true;
            m_len = static_cast<uint32_t>(ret);
        }
    } else {
        m_hasLen = false;
        m_len = 0;
    }

    m_keepAlive = p->m_requestHdr["Connection"_m].equalsIgnoreCase("keep-alive"_m);
}

float m::HTTPInputStream::progress() const
{
    if(!m_hasLen)
        return -1.0;

    float ret = static_cast<float>(m_pos) / static_cast<float>(m_len);
    if(ret < 0.0)
        return 0.0;
    else if(ret > 1.0)
        return 1.0;
    else
        return ret;
}

int m::HTTPOutputStream::write(const uint8_t *src, int sz)
{
    //TODO: Wrap multipart-fromdata here
    int ret = m_req->m_conn->send(src, sz);
    if(ret > 0)
        m_pos += ret;

    return ret;
}

#ifndef MGPCL_NO_SSL

unsigned long m::HTTPRequest::sslError() const
{
    if(m_conn == nullptr || !m_url.isValid() || m_url.protocol() == "https"_m)
        return 0;

    return static_cast<SSLSocket*>(m_conn)->lastSSLError();
}

m::String m::HTTPRequest::sslErrorString() const
{
    if(m_conn == nullptr || !m_url.isValid() || m_url.protocol() != "https"_m)
        return "not an https request"_m;

    return static_cast<SSLSocket*>(m_conn)->lastSSLErrorString();
}

#endif
