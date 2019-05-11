/* Copyright (C) 2019 BARBOTIN Nicolas
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
#include "Thread.h"
#include "Atomic.h"
#include "Mutex.h"
#include "HashMap.h"

#define M_HTTP_SERVER_RBUF_SZ 8192

namespace m
{
    class HTTPServer;

    class HTTPServerRequest
    {
        friend class HTTPServer;

    public:
        HTTPServerRequest() : m_method(kHRT_Get), m_queryLength(0), m_responseCode(500), m_responseLength(0), m_userdata(nullptr)
        {
        }

        HTTPRequestType method() const
        {
            return m_method;
        }

        const String &pathname() const
        {
            return m_pathname;
        }

        int numWildcards() const
        {
            return ~m_pathWildcards;
        }

        const String &wildcard(int idx) const
        {
            return m_pathWildcards[idx];
        }

        bool hasURLParam(const String &key) const
        {
            return m_queryParams.hasKey(key);
        }

        const String &urlParam(const String &key)
        {
            return m_queryParams[key];
        }

        bool hasRequestHeader(const String &key) const
        {
            return m_queryHeaders.hasKey(key);
        }

        const String &requestHeader(const String &key)
        {
            return m_queryHeaders[key];
        }

        uint64_t requestLength() const
        {
            return m_queryLength;
        }

        void setResponse(int code, const String &msg)
        {
            m_responseCode = code;
            m_responseMessage = msg;
        }

        int responseCode() const
        {
            return m_responseCode;
        }

        const String &responseMessage() const
        {
            return m_responseMessage;
        }

        void setResponseHeader(const String &key, const String &val)
        {
            m_responseHeaders[key] = val;
        }

        void setResponseHeader(const String &key, String &&val)
        {
            m_responseHeaders[key] = std::move(val);
        }

        bool hasResponseHeader(const String &key) const
        {
            return m_responseHeaders.hasKey(key);
        }

        const String &responseHeader(const String &key)
        {
            return m_responseHeaders[key];
        }

        void setResponseLength(uint64_t len)
        {
            m_responseLength = len;
            m_responseHeaders["Content-Length"] = String::fromUInteger(static_cast<uint32_t>(len));
        }

        uint64_t responseLength() const
        {
            return m_responseLength;
        }

        void setUserdata(void *ud)
        {
            m_userdata = ud;
        }

        void *userdata() const
        {
            return m_userdata;
        }

    private:
        //Query line data (method and path data)
        HTTPRequestType m_method;
        String m_pathname;
        List<String> m_pathWildcards;
        HashMap<String, String> m_queryParams;

        //Query headers
        HashMap<String, String, StringLowerHasher> m_queryHeaders;
        uint64_t m_queryLength;

        //Response line
        int m_responseCode;
        String m_responseMessage;

        //Response header
        HashMap<String, String, StringLowerHasher> m_responseHeaders;
        uint64_t m_responseLength;

        //Misc
        void *m_userdata;
    };

    class HTTPRequestHandler
    {
    public:
        virtual ~HTTPRequestHandler() {}
        virtual void beginRequest(HTTPServerRequest *req) = 0; //Called when server received headers
        virtual void receiveData(HTTPServerRequest *req, uint8_t *data, int sz) = 0;
        virtual void processRequest(HTTPServerRequest *req) = 0; //Called on data end; set response here
        virtual int sendData(HTTPServerRequest *req, uint8_t *dst, int dstSz) = 0; //Return output size
        virtual void finishRequest(HTTPServerRequest *req, bool success) = 0;
    };

    class HTTPServer
    {
        M_NON_COPYABLE(HTTPServer)

    public:
        HTTPServer();
        ~HTTPServer();

#ifndef MGPCL_NO_SSL
        bool enableSSL(const String &certFile, const String &keyFile);
        bool enableSSL(const SSLContext &ctx);
#endif

        bool start(const IPv4Address &listenAddr, int numThreads);
        void stop(bool graceful = true); //Right now graceful shutdown does nothing
        void set404Handler(HTTPRequestHandler *h);
        void bindHandler(const String &path, HTTPRequestHandler *h);

        void setInactivityTimeout(uint32_t ia)
        {
            m_inactivityTimeout = ia;
        }

        void setAccessLog(SSharedPtr<OutputStream> dst)
        {
            m_accessLog = dst;
        }

        uint32_t inactivityTimeout() const
        {
            return m_inactivityTimeout;
        }

        SSharedPtr<OutputStream> accessLog() const
        {
            return m_accessLog;
        }

    private:
        enum RequestPhase
        {
            kHRP_Handshake,
            kHRP_Read,
            kHRP_Write,
            kHRP_Shutdown
        };

        enum ReadPhase
        {
            kHRRP_QueryLine,
            kHRRP_Headers,
            kHRRP_Content
        };

        class Worker;

        class Client
        {
        public:
            Client(Worker *p, const IPv4Address &addr, TCPSocket *sock, bool ssl);
            ~Client();

            void comReady();
            void removeDueToError(const char *err);
            void onHeadersReceived();
            void startResponse();
            void finish();
            void stopClient();

#ifdef MGPCL_NO_SSL
            bool shouldRead() const
            {
                return m_phase == kHRP_Read;
            }

            bool shouldWrite() const
            {
                return m_phase == kHRP_Write;
            }
#else
            Client(Worker *p, const IPv4Address &addr, SSLSocket *sock, SSLWantedOperation handshakeOp);

            bool shouldRead() const
            {
                return m_isSSL ? (m_sslOP == kSWO_WantRead) : (m_phase == kHRP_Read);
            }

            bool shouldWrite() const
            {
                return m_isSSL ? (m_sslOP == kSWO_WantWrite) : (m_phase == kHRP_Write);
            }
#endif

            Worker *m_parent;

#ifndef MGPCL_NO_SSL
            bool m_isSSL;
#endif

            IPv4Address m_addr;
            TCPSocket *m_socket;

#ifndef MGPCL_NO_SSL
            SSLWantedOperation m_sslOP;
#endif

            RequestPhase m_phase;
            bool m_shouldRemove;
            uint32_t m_time;
            int m_lineLength;
            uint8_t m_recvBuf[M_HTTP_SERVER_RBUF_SZ];
            ReadPhase m_readPhase;
            bool m_writingHeaders;
            HTTPServerRequest *m_req;
            HTTPRequestHandler *m_handler;
            uint64_t m_remDataLen;
            String m_responseBuffer;
            int m_sentLinePos;
        };

        class Worker
        {
        public:
            Worker(HTTPServer *p) : m_parent(p)
            {
            }

            ~Worker();

            void run();
            static void run(void *data);
            void addClient(const IPv4Address &addr, TCPSocket &&cli);
            void stopClients();

            HTTPServer *m_parent;
            List<Client*> m_clients;
            List<Client*> m_selectedClients;
            Mutex m_clientLock;
            String m_accessBuf;
        };

        class Node
        {
        public:
            Node() : m_handler(nullptr), m_fallbackChild(nullptr) {}
            Node(const String &str) : m_match(str), m_handler(nullptr), m_fallbackChild(nullptr) {}
            ~Node();

            Node *child(const String &str);

            String m_match;
            Node *m_fallbackChild;
            List<Node*> m_children;
            HTTPRequestHandler *m_handler;
        };

        Worker *worker(int i) const
        {
            return static_cast<Worker*>(m_threadPool.userdata(i));
        }

        void dispatchClient(const IPv4Address &addr, TCPSocket &&cli);

        TCPSocket m_server;
        ThreadPool m_threadPool;
        Atomic m_running;
        Atomic m_dispatcher;

#ifndef MGPCL_NO_SSL
        SSLContext m_sslCtx;
#endif

        uint32_t m_inactivityTimeout;
        Node *m_root;
        HTTPRequestHandler *m_404handler;
        SSharedPtr<OutputStream> m_accessLog;
        Mutex m_accessLogLock;
    };

    class SimpleHTTPRequestHandler : public HTTPRequestHandler
    {
    public:
        SimpleHTTPRequestHandler() : m_maxRecvLen(65536) {}
        SimpleHTTPRequestHandler(uint64_t maxRecvLen) : m_maxRecvLen(maxRecvLen) {}

        void beginRequest(HTTPServerRequest *req) override;
        void receiveData(HTTPServerRequest *req, uint8_t *data, int sz) override;
        void processRequest(HTTPServerRequest *req) override;
        int sendData(HTTPServerRequest *req, uint8_t *dst, int dstSz) override;
        void finishRequest(HTTPServerRequest *req, bool success) override;

    protected:
        class SimpleUserdata
        {
        public:
            SimpleUserdata() : m_pos(0) {}
            virtual ~SimpleUserdata() {}

            int amountSent() const
            {
                return m_pos;
            }

            const String &requestContent() const
            {
                return m_recv;
            }

            const String &responseContent() const
            {
                return m_send;
            }

            void setResponse(const String &r)
            {
                m_send = r;
            }

            void receiveRequest(const uint8_t *data, int len);
            int sendResponse(uint8_t *dst, int sz);

        private:
            int m_pos;
            String m_recv;
            String m_send;
        };

        virtual SimpleUserdata *createUserdata();

    private:
        uint64_t m_maxRecvLen;
    };

    class StaticHTTPRequestHandler : public HTTPRequestHandler
    {
    public:
        StaticHTTPRequestHandler() : m_status(200), m_message("OK", 2), m_contentType("text/html") {}
        StaticHTTPRequestHandler(const String &data) : m_status(200), m_message("OK", 2), m_contentType("text/html"), m_data(data) {}
        StaticHTTPRequestHandler(String &&data) : m_status(200), m_message("OK", 2), m_contentType("text/html"), m_data(data) {}

        void beginRequest(HTTPServerRequest *req) override;
        void receiveData(HTTPServerRequest *req, uint8_t *data, int sz) override;
        void processRequest(HTTPServerRequest *req) override;
        int sendData(HTTPServerRequest *req, uint8_t *dst, int dstSz) override;
        void finishRequest(HTTPServerRequest *req, bool success) override;

        void setResponseCode(int code, const String &msg)
        {
            m_status = code;
            m_message = msg;
        }

        void setResponseCode(int code, String &&msg)
        {
            m_status = code;
            m_message = std::move(msg);
        }

        void setContentType(const String &ctype)
        {
            m_contentType = ctype;
        }

        void setContentType(String &&ctype)
        {
            m_contentType = std::move(ctype);
        }

        const String &contentType() const
        {
            return m_contentType;
        }

        void setData(const String &str)
        {
            m_data = str;
        }

        const String &data() const
        {
            return m_data;
        }

        int responseCode() const
        {
            return m_status;
        }

        const String &responseMessage() const
        {
            return m_message;
        }

    private:
        int m_status;
        String m_message;
        String m_contentType;
        String m_data;
    };

}
