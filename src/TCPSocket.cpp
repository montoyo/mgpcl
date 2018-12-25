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

#include "mgpcl/TCPSocket.h"
#include "mgpcl/Time.h"

bool m::TCPSocket::initialize()
{
    if(m_sock != INVALID_SOCKET)
        closesocket(m_sock);

    m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_sock == INVALID_SOCKET) {
        m_lastErr = inet::socketError();
        return false;
    }

    unsigned long val = 1;
    if(ioctlsocket(m_sock, FIONBIO, &val) == 0)
        return true;

    m_lastErr = inet::socketError();
    return false;
}

m::SocketConnectionError m::TCPSocket::connect(const IPv4Address &addr)
{
    if(m_sock == INVALID_SOCKET)
        return kSCE_NotInitialized;

    if(::connect(m_sock, reinterpret_cast<const struct sockaddr*>(addr.raw()), sizeof(struct sockaddr_in)) == 0)
        return kSCE_NoError;

    inet::SocketError err = inet::socketError();
    bool doesTimeout = m_connTimeout >= 0;

    fd_set wSet, eSet;
    struct timeval tv;

    if(err == inet::kSE_WouldBlock) {
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

        if(serror == 0)
            return kSCE_NoError; //Connected successfully

        err = inet::socketError(serror);
    }

    if(err == inet::kSE_TimedOut)
        return kSCE_TimedOut;

    m_lastErr = err;
    return kSCE_SocketError;
}

bool m::TCPSocket::bind(const IPv4Address &addr)
{
    if(m_sock == INVALID_SOCKET)
        return false;

    if(::bind(m_sock, reinterpret_cast<const struct sockaddr*>(addr.raw()), sizeof(struct sockaddr_in)) == 0)
        return true;

    m_lastErr = inet::socketError();
    return false;
}

bool m::TCPSocket::listen(int backlog)
{
    if(m_sock == INVALID_SOCKET)
        return false;

    if(::listen(m_sock, backlog) == 0)
        return true;

    m_lastErr = inet::socketError();
    return false;
}

m::TCPSocket m::TCPSocket::accept(IPv4Address &addr)
{
    m_lastErr = inet::kSE_NoError;

    fd_set set;
    struct sockaddr_in sain;
    struct timeval tv;

    socklen_t addrlen = sizeof(struct sockaddr_in);
    int remaining = m_connTimeout;

    SOCKET ret = ::accept(m_sock, reinterpret_cast<struct sockaddr*>(&sain), &addrlen);
    if(ret != INVALID_SOCKET) {
        if(addrlen == sizeof(struct sockaddr_in))
            addr = sain;

        //Success
        return TCPSocket(ret, m_connTimeout, m_readTimeout, m_writeTimeout);
    }

    inet::SocketError se = inet::socketError();
    bool doesTimeout = m_connTimeout >= 0;

    while(se == inet::kSE_WouldBlock && (!doesTimeout || remaining > 0)) {
        uint32_t begin = time::getTimeMsUInt();
        inet::fillTimeval(tv, static_cast<uint32_t>(remaining));

        FD_ZERO(&set);
        FD_SET(m_sock, &set);

        int s = select(m_sock + 1, &set, nullptr, nullptr, doesTimeout ? &tv : nullptr);
        if(s < 0) {
            //select should not fail
            m_lastErr = inet::kSE_UnknownError;
            return TCPSocket();
        } else if(s == 0)
            return TCPSocket(); //Invalid socket and inet::kSE_NoError means timed out

        addrlen = sizeof(struct sockaddr_in);
        ret = ::accept(m_sock, reinterpret_cast<struct sockaddr*>(&sain), &addrlen);

        if(ret != INVALID_SOCKET) {
            if(addrlen == sizeof(struct sockaddr_in))
                addr = sain;

            //Success
            return TCPSocket(ret, m_connTimeout, m_readTimeout, m_writeTimeout);
        }
        
        se = inet::socketError();
        remaining -= static_cast<int>(time::getTimeMsUInt() - begin);
    }

    if(se == inet::kSE_WouldBlock)
        m_lastErr = inet::kSE_NoError; //Timed out
    else
        m_lastErr = se;

    return TCPSocket();
}

m::TCPSocket &m::TCPSocket::operator = (TCPSocket &&src)
{
    if(m_sock != INVALID_SOCKET)
        closesocket(m_sock);

    m_lastErr = src.m_lastErr;
    m_sock = src.m_sock;
    m_connTimeout = src.m_connTimeout;
    m_readTimeout = src.m_readTimeout;
    m_writeTimeout = src.m_writeTimeout;

    src.m_sock = INVALID_SOCKET;
    return *this;
}

int m::TCPSocket::receive(uint8_t *dst, int sz)
{
    int ret = recv(m_sock, reinterpret_cast<char*>(dst), sz, 0);
    if(ret >= 0)
        return ret;

    struct timeval tv;
    fd_set set;

    inet::SocketError se = inet::socketError();
    int remaining = m_readTimeout;
    bool doesTimeout = m_readTimeout >= 0;

    while(se == inet::kSE_WouldBlock && (!doesTimeout || remaining > 0)) {
        uint32_t begin = time::getTimeMsUInt();
        inet::fillTimeval(tv, static_cast<uint32_t>(remaining));

        FD_ZERO(&set);
        FD_SET(m_sock, &set);

        int s = select(m_sock + 1, &set, nullptr, nullptr, doesTimeout ? &tv : nullptr);
        if(s < 0) {
            //select should not fail
            m_lastErr = inet::kSE_UnknownError;
            return -1;
        } else if(s == 0) {
            m_lastErr = inet::kSE_NoError;
            return -1; //Timed out
        }

        ret = recv(m_sock, reinterpret_cast<char*>(dst), sz, 0);
        if(ret >= 0)
            return ret;

        se = inet::socketError();
        remaining -= static_cast<int>(time::getTimeMsUInt() - begin);
    }

    if(se == inet::kSE_WouldBlock)
        m_lastErr = inet::kSE_NoError; //Timed out
    else
        m_lastErr = se;

    return -1;
}

int m::TCPSocket::send(const uint8_t *src, int sz)
{
    int ret = ::send(m_sock, reinterpret_cast<const char*>(src), sz, 0);
    if(ret >= 0)
        return ret;

    struct timeval tv;
    fd_set set;

    inet::SocketError se = inet::socketError();
    int remaining = m_writeTimeout;
    bool doesTimeout = m_writeTimeout >= 0;

    while(se == inet::kSE_WouldBlock && (!doesTimeout || remaining > 0)) {
        uint32_t begin = time::getTimeMsUInt();
        inet::fillTimeval(tv, static_cast<uint32_t>(remaining));

        FD_ZERO(&set);
        FD_SET(m_sock, &set);

        int s = select(m_sock + 1, nullptr, &set, nullptr, doesTimeout ? &tv : nullptr);
        if(s < 0) {
            //select should not fail
            m_lastErr = inet::kSE_UnknownError;
            return -1;
        } else if(s == 0) {
            m_lastErr = inet::kSE_NoError;
            return -1; //Timed out
        }

        ret = ::send(m_sock, reinterpret_cast<const char*>(src), sz, 0);
        if(ret >= 0)
            return ret;

        se = inet::socketError();
        remaining -= static_cast<int>(time::getTimeMsUInt() - begin);
    }

    if(se == inet::kSE_WouldBlock)
        m_lastErr = inet::kSE_NoError; //Timed out
    else
        m_lastErr = se;

    return -1;
}
