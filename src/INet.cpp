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

#define M_INET_SRC
#include "mgpcl/INet.h"
#include "mgpcl/Mem.h"

#ifndef MGPCL_NO_SSL
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#endif

m::inet::InitError m::inet::initialize()
{
#ifdef MGPCL_WIN
    WSADATA wsa;
    Mem::zero(&wsa, sizeof(WSADATA));

    wsa.wVersion = MAKEWORD(2, 2);
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);

    if(err == 0)
        return kIE_NoError;
    else if(err == WSASYSNOTREADY)
        return kIE_SystemNotReady;
    else if(err == WSAVERNOTSUPPORTED)
        return kIE_VersionNotSupported;
    else if(err == WSAEPROCLIM)
        return kIE_LimitReached;
    else
        return kIE_UnknownError; //WSAEINPROGRESS and WSAEFAULT should never happen
#else
    return kIE_NoError;
#endif
}

void m::inet::release()
{
#ifdef MGPCL_WIN
    WSACleanup();
#endif
}

m::inet::SocketError m::inet::socketError(int err)
{
#ifdef MGPCL_WIN
    switch(err) {
    case WSAEWOULDBLOCK:
        return kSE_WouldBlock;

    case WSAEADDRINUSE:
        return kSE_AddressInUse;

    case WSAECONNREFUSED:
        return kSE_ConnectionRefused;

    case WSAEISCONN:
        return kSE_SocketAlreadyConnected;

    case WSAETIMEDOUT:
        return kSE_TimedOut;

    case WSAENETDOWN:
    case WSAENETRESET:
    case WSAENETUNREACH:
        return kSE_NetworkUnreachable;

    default:
        return kSE_UnknownError;
    }
#else
    switch(err) {
#if EWOULDBLOCK != EAGAIN
    case EAGAIN:
#endif

    case EWOULDBLOCK:
    case EINPROGRESS:
        return kSE_WouldBlock;

    case EADDRINUSE:
        return kSE_AddressInUse;

    case ECONNREFUSED:
        return kSE_ConnectionRefused;

    case EISCONN:
        return kSE_SocketAlreadyConnected;

    case ETIMEDOUT:
        return kSE_TimedOut;

    case ENETDOWN:
    case ENETUNREACH:
        return kSE_NetworkUnreachable;

    default:
        return kSE_UnknownError;
    }
#endif
}

void m::inet::initSSL()
{
#ifndef MGPCL_NO_SSL
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    ERR_load_PEM_strings();
    ERR_load_X509_strings();
    OpenSSL_add_all_algorithms();
#endif
}
