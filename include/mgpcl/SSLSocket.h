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
#include "TCPSocket.h"
#include "SSLContext.h"
#include "Config.h"

#ifndef MGPCL_NO_SSL

namespace m
{
    enum SSLWantedOperation
    {
        kSWO_None = 0,
        kSWO_WantRead,
        kSWO_WantWrite
    };

    enum SSLAcceptError
    {
        kSAE_NoError = 0,
        kSAE_InvalidTCPSocket,
        kSAE_InvalidContext,
        kSAE_SSLError,
        kSAE_SSLHandshakeTimeout,
        kSAE_SocketError,
        kSAE_UnknownError
    };

    class SSLSocket : public TCPSocket
    {
    public:
        SSLSocket();
        SSLSocket(SSLSocket &&src);
        ~SSLSocket();

        SSLSocket &operator = (SSLSocket &&src);

        bool initialize(const SSLContext &ctx);
        bool initialize() override;
        SocketConnectionError connect(const IPv4Address &addr) override;
        int receive(uint8_t *dst, int sz) override;
        int send(const uint8_t *src, int sz) override;
        void close() override;
        bool close(bool sslShutdown);
        bool shutdown();

        /* initializeAndAccept() steals the socket from a TCPSocket instance
         * and waits for the SSL handshake. This method should be called right
         * after TCPSocket::accept().
         */
        SSLAcceptError initializeAndAccept(const SSLContext &ctx, TCPSocket &src);
        SSLAcceptError initializeAndAccept(const SSLContext &ctx, TCPSocket &&src);

        /* Resume method */
        SocketConnectionError resumeConnectHandshake();
        SSLAcceptError resumeAcceptHandshake();

        inet::SocketError lastError() const override
        {
            return m_lastErr;
        }

        unsigned int lastSSLError() const
        {
            return m_lastSSLErr;
        }

        String lastSSLErrorString() const;

        bool isValid() const override
        {
            return m_sock != INVALID_SOCKET && m_ssl_ != nullptr;
        }

        bool operator ! () const override
        {
            return m_sock == INVALID_SOCKET || m_ssl_ == nullptr;
        }

        const SSLContext &context() const
        {
            return m_ctx;
        }

        SSLWantedOperation lastWantedOperation()
        {
            return m_lastWantedOp;
        }

    private:
        template<class T> int sslRW(const T &data);
        SSLAcceptError initializeAndAccept(const SSLContext &ctx, SOCKET sock);

        SSLContext m_ctx;
        void *m_ssl_;
        unsigned int m_lastSSLErr;
        SSLWantedOperation m_lastWantedOp;
    };
}

#endif
