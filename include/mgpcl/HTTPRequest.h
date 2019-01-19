/* Copyright (C) 2017 BARBOTIN Nicolas
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

#pragma once
#include "HTTPCommons.h"
#include "SSLSocket.h"
#include "HTTPCookieJar.h"
#include "LineReader.h"

namespace m
{
    class HTTPInputStream;
    class HTTPOutputStream;
    class HTTPRequest
    {
        friend class HTTPInputStream;
        friend class HTTPOutputStream;

    public:
        HTTPRequest()
        {
            m_type = kHRT_Get;
            m_gotResponse = false;
            m_rcode = 0;
            m_doesOut = false;
            m_doesIn = true;
            m_followsLoc = false;
            m_jar = nullptr;
            m_conn = nullptr;
            m_lr.setLineEnding(LineEnding::CRLF);
            m_requestHdr["Connection"] = "close";
        }

        HTTPRequest(const URL &u)
        {
            m_url = u;
            m_type = kHRT_Get;
            m_gotResponse = false;
            m_rcode = 0;
            m_doesOut = false;
            m_doesIn = true;
            m_followsLoc = false;
            m_jar = nullptr;
            m_conn = nullptr;
            m_lr.setLineEnding(LineEnding::CRLF);
            m_requestHdr["Connection"] = "close";
        }

        HTTPRequest(const String &url) : m_url(url)
        {
            m_type = kHRT_Get;
            m_gotResponse = false;
            m_rcode = 0;
            m_doesOut = false;
            m_doesIn = true;
            m_followsLoc = false;
            m_jar = nullptr;
            m_conn = nullptr;
            m_lr.setLineEnding(LineEnding::CRLF);
            m_requestHdr["Connection"] = "close";
        }

        ~HTTPRequest()
        {
            if(m_conn != nullptr)
                delete m_conn;
        }

        void setRequestType(HTTPRequestType rt)
        {
            m_type = rt;
        }

        void setURL(const URL &url);

        void setRequestHeader(const String &prop, const String &val)
        {
            if(prop.lower() != "host")
                m_requestHdr[prop] = val;
        }

        /* Sets the request content type, shortcut for setRequestHeader("Content-Type", ct) */
        void setContentType(const String &ct)
        {
            m_requestHdr["Content-Type"] = ct;
        }

        /* Sets the request content length */
        void setContentLength(uint32_t clen)
        {
            m_requestHdr["Content-Length"] = String::fromUInteger(clen);
        }

        /* Checks if the response header contains the content length */
        bool hasContentLength() const
        {
            return m_responseHdr.hasKey("Content-Length");
        }

        /* Returns the response content length */
        uint32_t contentLength()
        {
            //The non-const version is supposed to be faster
            //since it doesn't copy the content length string...
            const String key("Content-Length");

            if(m_responseHdr.hasKey(key))
                return m_responseHdr[key].toUInteger();
            else
                return ~static_cast<uint32_t>(0);
        }

        /* Returns the response content length */
        uint32_t contentLength() const
        {
            const String key("Content-Length");

            if(m_responseHdr.hasKey(key))
                return m_responseHdr[key].toUInteger();
            else
                return ~static_cast<uint32_t>(0);
        }

        void setDoesOutput(bool o)
        {
            m_doesOut = o;
        }

        void setDoesInput(bool i)
        {
            m_doesIn = i;
        }

        HTTPRequestType requestType() const
        {
            return m_type;
        }

        const URL &url() const
        {
            return m_url;
        }

        const String &requestHeader(const String &prop)
        {
            return m_requestHdr[prop];
        }

        const String &responseHeader(const String &prop)
        {
            return m_responseHdr[prop];
        }

        Socket *socket()
        {
            return m_conn;
        }

        inet::SocketError socketError() const
        {
            return m_conn->lastError();
        }

        int responseCode() const
        {
            return m_rcode;
        }

        const String &status() const
        {
            return m_status;
        }

        bool doesOutput() const
        {
            //Should the user feed data before receiving data?
            return m_doesOut;
        }

        bool doesInput() const
        {
            //Does the server return data
            return m_doesIn;
        }

        HTTPCookieJar *cookieJar() const
        {
            return m_jar;
        }

        void setCookieJar(HTTPCookieJar *jar)
        {
            m_jar = jar;
        }

        bool hasCookieJar() const
        {
            return m_jar != nullptr;
        }

        void setKeepAlive(bool ka) {
            if(ka)
                m_requestHdr["Connection"] = "keep-alive";
            else
                m_requestHdr["Connection"] = "close";
        }

        /* Please note that followLocation won't work if doesOutput is set. */
        void setFollowsLocation(bool fl)
        {
            m_followsLoc = fl;
        }

        bool followsLocation() const
        {
            return m_followsLoc;
        }

        bool perform();
        bool receiveResponse();
        template<class RefCnt> SharedPtr<InputStream, RefCnt> inputStream(); //Response data stream
        template<class RefCnt> SharedPtr<OutputStream, RefCnt> outputStream(); //Request data stream

#ifndef MGPCL_NO_SSL
        unsigned long sslError() const;
        String sslErrorString() const;

        void setSSLContext(const SSLContext &ctx)
        {
            m_sslCtx = ctx;
        }

        SSLContext &sslContext()
        {
            return m_sslCtx;
        }

        const SSLContext &sslContext() const
        {
            return m_sslCtx;
        }
#endif

    private:
        bool receiveResponse(bool keepAlive);

        URL m_url;
        HTTPRequestType m_type;
        HTTPCookieJar *m_jar;
        HashMap<String, String, StringLowerHasher> m_requestHdr;
        HashMap<String, String, StringLowerHasher> m_responseHdr;
        Socket *m_conn;
        LineReader m_lr;
        String m_status;

#ifndef MGPCL_NO_SSL
        SSLContext m_sslCtx;
#endif

        int m_rcode;
        bool m_gotResponse;
        bool m_doesOut;
        bool m_doesIn;
        bool m_followsLoc;
    };

    class HTTPInputStream : public InputStream
    {
        friend class HTTPRequest;

    public:
        ~HTTPInputStream() override
        {
        }

        int read(uint8_t *dst, int sz) override;
        float progress() const; //Returns -1.0 if Content-Length was not provided or is -1, otherwise, it returns a value from 0.0 to 1.0

        uint64_t pos() override
        {
            return static_cast<uint64_t>(m_pos);
        }

        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
        {
            return false;
        }

        bool seekSupported() const override
        {
            return false;
        }

        void close() override
        {
        }

    private:
        HTTPInputStream()
        {
        }

        HTTPInputStream(HTTPRequest *p);

        HTTPRequest *m_req;
        uint32_t m_pos;
        bool m_hasLen;
        uint32_t m_len;
        bool m_keepAlive;
    };

    //Doesn't need to buffer, Nagle's algorithm is here for that
    class HTTPOutputStream : public OutputStream
    {
        friend class HTTPRequest;

    public:
        ~HTTPOutputStream() override
        {
        }

        int write(const uint8_t *src, int sz) override;

        uint64_t pos() override
        {
            return static_cast<uint64_t>(m_pos);
        }

        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
        {
            return false;
        }

        bool seekSupported() const override
        {
            return false;
        }

        bool flush() override
        {
            return true;
        }

        void close() override
        {
        }

    private:
        HTTPOutputStream()
        {
        }

        HTTPOutputStream(HTTPRequest *req)
        {
            m_req = req;
            m_pos = 0;
        }

        HTTPRequest *m_req;
        uint32_t m_pos;
    };

    template<class RefCnt> SharedPtr<InputStream, RefCnt> HTTPRequest::inputStream()
    {
        if(!m_doesIn || (m_doesOut && !receiveResponse()))
            return SharedPtr<InputStream, RefCnt>(); //return null

        return SharedPtr<InputStream, RefCnt>(new HTTPInputStream(this));
    }

    template<class RefCnt> SharedPtr<OutputStream, RefCnt> HTTPRequest::outputStream()
    {
        if(!m_doesOut)
            return SharedPtr<OutputStream, RefCnt>(); //return null

        return SharedPtr<OutputStream, RefCnt>(new HTTPOutputStream(this));
    }

}
