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
#include "INet.h"
#include "IPv4Address.h"
#include "IOStream.h"
#include "SharedPtr.h"

namespace m
{
    enum SocketConnectionError
    {
        kSCE_NoError = 0,         //Everything went fine
        kSCE_NotInitialized,      //Forgot to call TCPSocket::initialize()
        kSCE_TimedOut,            //connectionTimeout elapsed
        kSCE_SocketError,         //Use TCPSocket::lastError() for more info
        kSCE_SSLError,            //Only for SSLSocket, internal SSL error (see SSLSocket::lastSSLError())
        kSCE_SSLHandshakeTimeout, //Only for SSLSocket, the handshake timed out (can be resumed with SSLSocket::resumeConnectHandshake())
        kSCE_UnknownError         //Dunno what happened
    };
    
    class Socket
    {
    public:
        Socket()
        {
            m_connTimeout = 10000;
            m_readTimeout = 2000;
            m_writeTimeout = 2000;
        }

        virtual ~Socket()
        {
        }

        virtual bool initialize() = 0;
        virtual SocketConnectionError connect(const IPv4Address &addr) = 0;
        virtual int receive(uint8_t *dst, int sz) = 0;
        virtual int send(const uint8_t *src, int sz) = 0;
        virtual void close() = 0;
        virtual bool isValid() const = 0;
        virtual bool operator ! () const = 0;
        virtual inet::SocketError lastError() const = 0;

        void setConnectionTimeout(int val)
        {
            m_connTimeout = val;
        }

        void setAcceptTimeout(int val)
        {
            m_connTimeout = val;
        }

        void setReadTimeout(int val)
        {
            m_readTimeout = val;
        }

        void setWriteTimeout(int val)
        {
            m_writeTimeout = val;
        }

        void setReadAndWriteTimeouts(int r, int w)
        {
            m_readTimeout = r;
            m_writeTimeout = w;
        }

        void setReadAndWriteTimeouts(int to)
        {
            m_readTimeout = to;
            m_writeTimeout = to;
        }

        int connectionTimeout() const
        {
            return m_connTimeout;
        }

        int acceptTimeout() const
        {
            return m_connTimeout;
        }

        int readTimeout() const
        {
            return m_readTimeout;
        }

        int writeTimeout() const
        {
            return m_writeTimeout;
        }

        template<class RefCnt> SharedPtr<InputStream, RefCnt> inputStream();
        template<class RefCnt> SharedPtr<OutputStream, RefCnt> outputStream();

    protected:
        int m_connTimeout;
        int m_readTimeout;
        int m_writeTimeout;
    };

    class SSLSocket;
    class TCPSocketSet;

    /* TCPSocket can be blocking, non-blocking, or timeout driven.
     * Blocking:       by setting timeouts to a negative value, all functions will wait indefinitely until their job is done
     * Non-blocking:   by setting timeouts to zero, all functions will return immediately. If a function didn't have time to
     *                 achieve its mission, it will return a negative value (or, for accept(), an invalid socket) and set the
     *                 last socket error to inet::kSE_NoError.
     *                 Note that setting the connect timeout (on a client socket) to 0 (= non-blocking) makes no sense and is
     *                 very likely to cause issues.
     * Timeout driven: (default) simply set the timeouts to a positive value
     */
    class TCPSocket : public Socket
    {
        friend class SSLSocket;
        friend class TCPSocketSet;

    public:
        TCPSocket()
        {
            m_lastErr = inet::kSE_NoError;
            m_sock = INVALID_SOCKET;
        }

        TCPSocket(TCPSocket &&src) : Socket(src)
        {
            m_lastErr = src.m_lastErr;
            m_sock = src.m_sock;
            src.m_sock = INVALID_SOCKET;
        }

        virtual ~TCPSocket()
        {
            if(m_sock != INVALID_SOCKET)
                closesocket(m_sock);
        }

        void close() override
        {
            if(m_sock != INVALID_SOCKET) {
                closesocket(m_sock);
                m_sock = INVALID_SOCKET;
            }
        }

        TCPSocket &operator = (TCPSocket &&src);

        bool initialize() override;
        SocketConnectionError connect(const IPv4Address &addr) override;
        bool bind(const IPv4Address &addr);
        bool listen(int backlog = SOMAXCONN);

        /*
         * Use client.isValid() to check if it worked (or the '!' operator)
         * If the resulting socket is invalid, use server.lastError()
         * in order to determine what's going on. If it's inet::kSE_NoError,
         * then accept() timed out.
         */
        TCPSocket accept(IPv4Address &addr);

        int receive(uint8_t *dst, int sz) override;
        int send(const uint8_t *src, int sz) override;

        bool isValid() const override
        {
            return m_sock != INVALID_SOCKET;
        }

        bool operator ! () const override
        {
            return m_sock == INVALID_SOCKET;
        }

        inet::SocketError lastError() const override
        {
            return m_lastErr;
        }

        SOCKET raw()
        {
            //Not const because you can do whatever you want with this
            return m_sock;
        }

    protected:
        TCPSocket(SOCKET s, int c, int r, int w)
        {
            m_lastErr = inet::kSE_NoError;
            m_sock = s;
            m_connTimeout = c;
            m_readTimeout = r;
            m_writeTimeout = w;
        }

        TCPSocket(const TCPSocket &src)
        {
        }

        inet::SocketError m_lastErr;
        SOCKET m_sock;
    };

    class TCPSocketSet
    {
        friend class TCPSocket;

    public:
        TCPSocketSet()
        {
            FD_ZERO(&m_data);

#ifdef MGPCL_LINUX
            m_maxFD = 0;
#endif
        }

        TCPSocketSet &add(const TCPSocket &s)
        {
            mDebugAssert(s.m_sock != INVALID_SOCKET, "trying to add invalid socket to TCPSocketSet");
            FD_SET(s.m_sock, &m_data);

#ifdef MGPCL_LINUX
            if(s.m_sock > m_maxFD)
                m_maxFD = s.m_sock;
#endif

            return *this;
        }

        bool isSet(const TCPSocket &s) const
        {
            return FD_ISSET(s.m_sock, const_cast<fd_set*>(&m_data)) != 0;
        }

        static int waitForSets(TCPSocketSet *rd, TCPSocketSet *wr, TCPSocketSet *err, int timeoutMS)
        {
            struct timeval tv;
            if(timeoutMS >= 0)
                inet::fillTimeval(tv, static_cast<uint32_t>(timeoutMS));

#ifdef MGPCL_LINUX
            fd_set *rdSet, *wrSet, *errSet;
            int maxFD;

            if(rd == nullptr) {
                rdSet = nullptr;
                maxFD = 0;
            } else {
                rdSet = &rd->m_data;
                maxFD = rd->m_maxFD;
            }

            if(wr == nullptr)
                wrSet = nullptr;
            else {
                wrSet = &wr->m_data;

                if(wr->m_maxFD > maxFD)
                    maxFD = wr->m_maxFD;
            }

            if(err == nullptr)
                errSet = nullptr;
            else {
                errSet = &err->m_data;

                if(err->m_maxFD > maxFD)
                    maxFD = err->m_maxFD;
            }

            return select(maxFD + 1, rdSet, wrSet, errSet, (timeoutMS >= 0) ? &tv : nullptr);
#else
            fd_set *rdSet  = (rd  == nullptr) ? nullptr :  &rd->m_data;
            fd_set *wrSet  = (wr  == nullptr) ? nullptr :  &wr->m_data;
            fd_set *errSet = (err == nullptr) ? nullptr : &err->m_data;

            return select(0, rdSet, wrSet, errSet, (timeoutMS >= 0) ? &tv : nullptr);
#endif
        }

    private:
        fd_set m_data;
#ifdef MGPCL_LINUX
        int m_maxFD;
#endif
    };

    class SocketIStream : public InputStream
    {
        friend class Socket;

    public:
        ~SocketIStream() override
        {
        }

        int read(uint8_t *dst, int sz) override
        {
            return m_ptr->receive(dst, sz);
        }

        uint64_t pos() override
        {
            return 0;
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

        Socket *socket()
        {
            return m_ptr;
        }

    private:
        SocketIStream()
        {
            std::abort();
        }

        SocketIStream(Socket *sock)
        {
            m_ptr = sock;
        }

        Socket *m_ptr;
    };

    class SocketOStream : public OutputStream
    {
        friend class Socket;

    public:
        ~SocketOStream() override
        {
        }

        int write(const uint8_t *src, int sz) override
        {
            return m_ptr->send(src, sz);
        }

        uint64_t pos() override
        {
            return 0;
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

        Socket *socket()
        {
            return m_ptr;
        }

    private:
        SocketOStream()
        {
            std::abort();
        }

        SocketOStream(Socket *sock)
        {
            m_ptr = sock;
        }

        Socket *m_ptr;
    };

    template<class RefCnt> SharedPtr<InputStream, RefCnt> Socket::inputStream()
    {
        return SharedPtr<InputStream, RefCnt>(new SocketIStream(this));
    }

    template<class RefCnt> SharedPtr<OutputStream, RefCnt> Socket::outputStream()
    {
        return SharedPtr<OutputStream, RefCnt>(new SocketOStream(this));
    }

}
