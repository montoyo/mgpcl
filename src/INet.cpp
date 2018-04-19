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
#include "mgpcl/IOStream.h"

#ifndef MGPCL_NO_SSL
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/err.h>

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
 //Version 1.1.x support
#define M_CRYPTO_11
#endif

static BIO_METHOD *g_bioMethod;
#endif

m::inet::InitError m::inet::initialize()
{
#ifdef MGPCL_WIN
    WSADATA wsa;
    mem::zero(&wsa, sizeof(WSADATA));

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

#ifdef MGPCL_NO_SSL

void m::inet::initSSL()
{
}

void *m::inet::makeBIO(m::InputStream *is)
{
    return nullptr;
}

#else

class BIOInputStream
{
public:
    BIOInputStream(m::InputStream *is_) : is(is_), hasUndo(false)
    {
    }

    int read(char *data, int len)
    {
        if(hasUndo) {
            *data = undo;
            hasUndo = false;

            if(--len <= 0)
                return 1;
        }

        return is->read(reinterpret_cast<uint8_t*>(data), len);
    }

    int getc(char *dst)
    {
        if(hasUndo) {
            *dst = undo;
            hasUndo = false;
            return 1;
        }

        return is->read(reinterpret_cast<uint8_t*>(dst), 1);
    }

    void revert(char c)
    {
        hasUndo = true;
        undo = c;
    }

    static BIOInputStream *get(BIO *bio)
    {
#ifdef M_CRYPTO_11
        return static_cast<BIOInputStream*>(BIO_get_data(bio));
#else
        return static_cast<BIOInputStream*>(bio->ptr);
#endif
    }

    m::InputStream *is;
    bool hasUndo;
    char undo;
};

static int bioRead(BIO *bio, char *dst, int len)
{
    return BIOInputStream::get(bio)->read(dst, len);
}

static int bioGets(BIO *bio, char *dst, int len)
{
    BIOInputStream *is = BIOInputStream::get(bio);
    bool err = false;
    int pos;

    for(pos = 0; pos < len - 1; pos++) {
        int ret = is->getc(dst);
        if(ret <= 0) {
            if(ret < 0)
                err = true;

            break;
        }

        if(*dst == '\r') {
            *dst = '\n';

            char nxt;
            if(is->getc(&nxt) > 0 && nxt != '\n')
                is->revert(nxt);
        }

        if(*(dst++) == '\n')
            break;
    }

    *dst = 0;
    if(pos > 0)
        return pos;
    else
        return err ? -1 : 0;
}

static int bioDestroy(BIO *b)
{
    delete BIOInputStream::get(b);
    return 0;
}

void m::inet::initSSL()
{
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    ERR_load_PEM_strings();
    ERR_load_X509_strings();
    OpenSSL_add_all_algorithms();

#ifdef M_CRYPTO_11
    g_bioMethod = BIO_meth_new(BIO_get_new_index(), "MGPCL_BIO");
    BIO_meth_set_read(g_bioMethod, bioRead);
    BIO_meth_set_gets(g_bioMethod, bioGets);
    BIO_meth_set_destroy(g_bioMethod, bioDestroy);
#else
    g_bioMethod = new BIO_METHOD;
    mem::zero(g_bioMethod, sizeof(BIO_METHOD));
    g_bioMethod->type = 42 | BIO_TYPE_SOURCE_SINK;
    g_bioMethod->name = "MGPCL_BIO";
    g_bioMethod->bread = bioRead;
    g_bioMethod->bgets = bioGets;
    g_bioMethod->destroy = bioDestroy;
#endif
}

void *m::inet::makeBIO(m::InputStream *is)
{
    BIO *ret = BIO_new(g_bioMethod);

#ifdef M_CRYPTO_11
    BIO_set_data(ret, new BIOInputStream(is));
#else
    ret->ptr = new BIOInputStream(is);
#endif

    return ret;
}

#endif
