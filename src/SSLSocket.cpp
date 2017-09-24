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

#include "mgpcl/SSLSocket.h"

#ifndef MGPCL_NO_SSL
#include "mgpcl/Time.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

#define m_ssl (*reinterpret_cast<SSL**>(&m_ssl_))

m::SSLSocket::SSLSocket()
{
    m_sock = INVALID_SOCKET;
    m_lastErr = inet::kSE_NoError;
    m_lastSSLErr = 0;
    m_ssl = nullptr;
}

m::SSLSocket::SSLSocket(SSLSocket &&src) : Socket(src), m_ctx(std::move(src.m_ctx))
{
    m_sock = src.m_sock;
    m_lastErr = src.m_lastErr;
    m_lastSSLErr = src.m_lastSSLErr;
    m_ssl_ = src.m_ssl_;

    src.m_sock = INVALID_SOCKET;
    src.m_ssl_ = nullptr;
}

bool m::SSLSocket::initialize()
{
    SSLContext ctx(kSCM_v23Method);
    return initialize(ctx);
}

bool m::SSLSocket::initialize(const SSLContext &ctx)
{
    if(m_sock != INVALID_SOCKET)
        closesocket(m_sock);

    if(m_ssl != nullptr)
        SSL_free(m_ssl);

    if(!ctx.isValid())
        return false;

    m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_sock == INVALID_SOCKET) {
        m_lastErr = inet::socketError();
        return false;
    }

    //Switch on non blocking IO to manage timeout
    unsigned long val = 1;
    if(ioctlsocket(m_sock, FIONBIO, &val) != 0) {
        m_lastErr = inet::socketError();
        return false;
    }

    m_ctx = ctx;
    m_ssl = SSL_new(static_cast<SSL_CTX*>(ctx.raw()));
    return m_ssl != nullptr;
}

class SSLRWRead
{
public:
    uint8_t *dst;
    int sz;

    SSLRWRead(uint8_t *pdst, int psz)
    {
        dst = pdst;
        sz = psz;
    }

    int apply(SSL *ssl) const
    {
        return SSL_read(ssl, dst, sz);
    }

    static bool isOK(int result)
    {
        return result >= 0;
    }
};

class SSLRWWrite
{
public:
    const uint8_t *src;
    int sz;

    SSLRWWrite(const uint8_t *psrc, int psz)
    {
        src = psrc;
        sz = psz;
    }

    int apply(SSL *ssl) const
    {
        return SSL_write(ssl, src, sz);
    }

    static bool isOK(int result)
    {
        return result >= 0;
    }
};

class SSLRWConnect
{
public:
    static int apply(SSL *ssl)
    {
        return SSL_connect(ssl);
    }

    static bool isOK(int result)
    {
        return result == 1;
    }
};

class SSLRWShutdown
{
public:
    static int apply(SSL *ssl)
    {
        return SSL_shutdown(ssl);
    }

    static bool isOK(int result)
    {
        return result == 1;
    }
};

template<class T> int m::SSLSocket::sslRW(const T &data)
{
    int ret = data.apply(m_ssl);
    if(data.isOK(ret))
        return ret;

    int err = SSL_get_error(m_ssl, ret);
    if(err == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE) {
        struct timeval tv;
        fd_set set;

        int remaining = (err == SSL_ERROR_WANT_READ) ? m_readTimeout : m_writeTimeout;
        bool doesTimeout = (remaining >= 0);

        while((err == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE) && (!doesTimeout || remaining > 0)) {
            uint32_t begin = time::getTimeMsUInt();
            inet::fillTimeval(tv, static_cast<uint32_t>(remaining));

            FD_ZERO(&set);
            FD_SET(m_sock, &set);

            int s = select(m_sock + 1, err == SSL_ERROR_WANT_READ ? &set : nullptr, err == SSL_ERROR_WANT_WRITE ? &set : nullptr, nullptr, doesTimeout ? &tv : nullptr);
            if(s < 0) {
                //select should not fail
                m_lastSSLErr = 0;
                m_lastErr = inet::kSE_UnknownError;
                return -1;
            } else if(s == 0) {
                m_lastSSLErr = 0;
                m_lastErr = inet::kSE_NoError;
                return -1; //Timed out
            }

            ret = data.apply(m_ssl);
            if(data.isOK(ret))
                return ret;

            err = SSL_get_error(m_ssl, ret);
            remaining -= static_cast<int>(time::getTimeMsUInt() - begin);
        }

        if(remaining <= 0) {
            m_lastSSLErr = 0;
            m_lastErr = inet::kSE_NoError; //Timed out
            return -1;
        }
    }

    //ERROR
    m_lastSSLErr = ERR_get_error();
    m_lastErr = inet::kSE_NoError;
    return -1;
}

m::SSLSocket::~SSLSocket()
{
    if(m_ssl != nullptr) {
        SSLRWShutdown crap;
        sslRW<SSLRWShutdown>(crap);
        SSL_free(m_ssl);
    }

    if(m_sock != INVALID_SOCKET)
        closesocket(m_sock);
}

m::SocketConnectionError m::SSLSocket::connect(const IPv4Address &addr)
{
    if(m_sock == INVALID_SOCKET || m_ssl == nullptr)
        return kSCE_NotInitialized;

    //Establish connection
    if(::connect(m_sock, reinterpret_cast<const struct sockaddr*>(addr.raw()), sizeof(struct sockaddr_in)) != 0) {
        inet::SocketError err = inet::socketError();

        if(err == inet::kSE_WouldBlock) {
            bool doesTimeout = m_connTimeout >= 0;
            fd_set wSet, eSet;
            struct timeval tv;

            FD_ZERO(&wSet);
            FD_ZERO(&eSet);
            FD_SET(m_sock, &wSet);
            FD_SET(m_sock, &eSet);

            inet::fillTimeval(tv, static_cast<uint32_t>(m_connTimeout));

            int cnt = select(m_sock + 1, nullptr, &wSet, &eSet, doesTimeout ? &tv : nullptr);
            if(cnt < 0)
                return kSCE_UnknownError; //Select should not fail
            else if(cnt == 0)
                return kSCE_TimedOut;

            int serror;
            socklen_t slen = sizeof(int);
            if(getsockopt(m_sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&serror), &slen) != 0)
                return kSCE_UnknownError; //getsockopt should not fail

            if(serror != 0) {
                err = inet::socketError(serror);
                if(err == inet::kSE_TimedOut)
                    return kSCE_TimedOut;

                m_lastErr = err;
                return kSCE_SocketError;
            }
        } else if(err == inet::kSE_TimedOut)
            return kSCE_TimedOut;
        else {
            m_lastErr = err;
            return kSCE_SocketError;
        }
    }

    if(SSL_set_fd(m_ssl, static_cast<int>(m_sock)) == 0)
        return kSCE_UnknownError;

    /*BIO_set_nbio(SSL_get_wbio(m_ssl), 1);
    BIO_set_nbio(SSL_get_rbio(m_ssl), 1);*/

    //SSL handshake
    SSLRWConnect crap;
    if(sslRW<SSLRWConnect>(crap) <= 0)
        return m_lastSSLErr == SSL_ERROR_NONE ? kSCE_SocketError : kSCE_SSLError;

    return kSCE_NoError;
}

int m::SSLSocket::receive(uint8_t *dst, int sz)
{
    SSLRWRead args(dst, sz);
    return sslRW<SSLRWRead>(args);
}

int m::SSLSocket::send(const uint8_t *src, int sz)
{
    SSLRWWrite args(src, sz);
    return sslRW<SSLRWWrite>(args);
}

m::SSLSocket &m::SSLSocket::operator = (SSLSocket &&src)
{
    if(m_ssl != nullptr) {
        SSLRWShutdown crap;
        sslRW<SSLRWShutdown>(crap);
        SSL_free(m_ssl);
    }

    if(m_sock != INVALID_SOCKET)
        closesocket(m_sock);

    m_sock = src.m_sock;
    m_ctx = std::move(src.m_ctx);
    m_lastErr = src.m_lastErr;
    m_lastSSLErr = src.m_lastSSLErr;
    m_ssl_ = src.m_ssl_;
    m_connTimeout = src.m_connTimeout;
    m_readTimeout = src.m_readTimeout;
    m_writeTimeout = src.m_writeTimeout;

    src.m_sock = INVALID_SOCKET;
    src.m_ssl_ = nullptr;
    return *this;
}

void m::SSLSocket::close()
{
    if(m_ssl != nullptr) {
        SSLRWShutdown crap;
        sslRW<SSLRWShutdown>(crap);
        SSL_free(m_ssl);
        m_ssl = nullptr;
    }

    if(m_sock != INVALID_SOCKET) {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
    }
}

m::String m::SSLSocket::lastSSLErrorString() const
{
    String ret(119);
    ERR_error_string(m_lastSSLErr, ret.begin());
    return ret;
}

#endif
