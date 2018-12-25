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
#include "Config.h"
#include <cstdint>

#ifdef M_INET_SRC
#define M_INET_PREFIX
#else
#define M_INET_PREFIX extern
#endif

#ifdef MGPCL_WIN
#include <WinSock2.h>
#include <WS2tcpip.h>
typedef int socklen_t;
#define select(ndfs, rdfs, wrfs, errfs, to) ::select(0, rdfs, wrfs, errfs, to)
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
typedef int SOCKET;
#define INVALID_SOCKET static_cast<SOCKET>(-1)
#define SOCKET_ERROR -1
#define closesocket ::close
#define ioctlsocket ::ioctl
#endif

namespace m
{
    class InputStream;

    namespace inet
    {
        enum InitError
        {
            kIE_NoError = 0,
            kIE_SystemNotReady,
            kIE_VersionNotSupported,
            kIE_LimitReached,
            kIE_UnknownError
        };

        enum SocketError
        {
            kSE_NoError = 0,
            kSE_WouldBlock,
            kSE_AddressInUse,
            kSE_ConnectionRefused,
            kSE_SocketAlreadyConnected,
            kSE_TimedOut,
            kSE_NetworkUnreachable,
            kSE_SSLError,
            kSE_UnknownError
        };

        M_INET_PREFIX InitError initialize();
        M_INET_PREFIX void release();
        M_INET_PREFIX void initSSL();
        M_INET_PREFIX void *makeBIO(m::InputStream *is);

#ifdef MGPCL_WIN
        M_INET_PREFIX SocketError socketError(int err = WSAGetLastError());
#else
        M_INET_PREFIX SocketError socketError(int err = errno);
#endif

        inline void fillTimeval(struct timeval &tv, uint32_t ms)
        {
            tv.tv_sec = ms / 1000;
            tv.tv_usec = (ms % 1000) * 1000;
        }
    }
}
