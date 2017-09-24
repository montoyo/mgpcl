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

#include "mgpcl/SHA.h"

#ifndef MGPCL_NO_SSL
#include <openssl/evp.h>

#define m_ctx (*reinterpret_cast<EVP_MD_CTX**>(&m_ctx_))

static const EVP_MD *g_shaEVP(m::SHAVersion ver)
{
    switch(ver) {
    /*case m::kSHAV_Sha0:
        return EVP_sha();*/

    case m::kSHAV_Sha1:
        return EVP_sha1();

    case m::kSHAV_Sha224:
        return EVP_sha224();

    case m::kSHAV_Sha256:
        return EVP_sha256();

    case m::kSHAV_Sha384:
        return EVP_sha384();

    case m::kSHAV_Sha512:
        return EVP_sha512();

    default:
        return nullptr;
    }
}

m::SHA::SHA()
{
    m_ver = kSHAV_None;
    m_ctx = nullptr;
}

m::SHA::SHA(SHAVersion ver)
{
    const EVP_MD *alg = g_shaEVP(ver);

    if(alg == nullptr) {
        m_ver = kSHAV_None;
        m_ctx = nullptr;
    } else {
        m_ver = ver;
        m_ctx = EVP_MD_CTX_create();
        EVP_DigestInit(m_ctx, alg);
    }
}

m::SHA::SHA(const SHA &src)
{
    if(src.m_ctx_ == nullptr) {
        m_ver = kSHAV_None;
        m_ctx = nullptr;
    } else {
        m_ver = src.m_ver;
        m_ctx = EVP_MD_CTX_create();
        EVP_MD_CTX_copy(m_ctx, static_cast<const EVP_MD_CTX*>(src.m_ctx_));
    }
}

m::SHA::SHA(SHA &&src)
{
    m_ver = src.m_ver;
    m_ctx_ = src.m_ctx_;
    src.m_ctx_ = nullptr;
}

m::SHA::~SHA()
{
    if(m_ctx != nullptr)
        EVP_MD_CTX_destroy(m_ctx);
}

bool m::SHA::init(SHAVersion ver)
{
    if(m_ctx != nullptr)
        EVP_MD_CTX_destroy(m_ctx);

    const EVP_MD *alg = g_shaEVP(ver);

    if(alg == nullptr) {
        m_ver = kSHAV_None;
        m_ctx = nullptr;
        return false;
    } else {
        m_ver = ver;
        m_ctx = EVP_MD_CTX_create();
        EVP_DigestInit(m_ctx, alg);
        return true;
    }
}

void m::SHA::reset()
{
    mDebugAssert(m_ctx != nullptr, "forgot to initialize SHA");
    EVP_DigestInit(m_ctx, EVP_MD_CTX_md(m_ctx));
}

void m::SHA::update(const uint8_t *src, uint32_t sz)
{
    mDebugAssert(m_ctx != nullptr, "forgot to initialize SHA");
    mDebugAssert(src != nullptr, "passed nullptr to SHA::update()");
    EVP_DigestUpdate(m_ctx, src, static_cast<size_t>(sz));
}

void m::SHA::update(const char *str, int len)
{
    mDebugAssert(m_ctx != nullptr, "forgot to initialize SHA");
    mDebugAssert(str != nullptr, "passed nullptr to SHA::update()");

    if(len < 0) {
        len = 0;

        while(str[len] != 0)
            len++;
    }

    EVP_DigestUpdate(m_ctx, str, static_cast<size_t>(len));
}

void m::SHA::update(const String &str)
{
    mDebugAssert(m_ctx != nullptr, "forgot to initialize SHA");
    EVP_DigestUpdate(m_ctx, str.raw(), static_cast<size_t>(str.length()));
}

bool m::SHA::digest(uint8_t *dst, uint32_t sz)
{
    mDebugAssert(m_ctx != nullptr, "forgot to initialize SHA");
    mDebugAssert(dst != nullptr, "passed nullptr to SHA::digest()");

    if(sz < static_cast<uint32_t>(EVP_MD_CTX_size(m_ctx)))
        return false;

    EVP_DigestFinal(m_ctx, dst, nullptr);
    return true;
}

void m::SHA::digest(String &dst)
{
    mDebugAssert(m_ctx != nullptr, "forgot to initialize SHA");

    uint8_t digest[EVP_MAX_MD_SIZE];
    uint32_t len = EVP_MAX_MD_SIZE;

    EVP_DigestFinal(m_ctx, digest, &len);
    hexString(digest, len, dst);
}

uint8_t *m::SHA::digest()
{
    mDebugAssert(m_ctx != nullptr, "forgot to initialize SHA");

    uint8_t *digest = new uint8_t[EVP_MD_CTX_size(m_ctx)];
    EVP_DigestFinal(m_ctx, digest, nullptr);
    return digest;
}

uint32_t m::SHA::digestSize() const
{
    const EVP_MD_CTX *ctx = static_cast<const EVP_MD_CTX*>(m_ctx_);
    if(ctx == nullptr)
        return 0;

    return static_cast<uint32_t>(EVP_MD_CTX_size(ctx));
}

uint32_t m::SHA::digestSize(SHAVersion ver)
{
    const EVP_MD *alg = g_shaEVP(ver);
    if(alg == nullptr)
        return 0;

    return static_cast<uint32_t>(EVP_MD_size(alg));
}

m::SHA &m::SHA::operator = (const SHA &src)
{
    if(m_ctx_ == src.m_ctx_)
        return *this;

    if(m_ctx != nullptr)
        EVP_MD_CTX_destroy(m_ctx);

    if(src.m_ctx_ == nullptr) {
        m_ver = kSHAV_None;
        m_ctx = nullptr;
    } else {
        m_ver = src.m_ver;
        m_ctx = EVP_MD_CTX_create();
        EVP_MD_CTX_copy(m_ctx, static_cast<const EVP_MD_CTX*>(src.m_ctx_));
    }

    return *this;
}

m::SHA &m::SHA::operator = (SHA &&src)
{
    if(m_ctx != nullptr)
        EVP_MD_CTX_destroy(m_ctx);

    m_ver = src.m_ver;
    m_ctx_ = src.m_ctx_;
    src.m_ctx_ = nullptr;
    return *this;
}

m::String m::SHA::quick(SHAVersion ver, const uint8_t *data, uint32_t len)
{
    String ret;
    const EVP_MD *alg = g_shaEVP(ver);

    if(alg == nullptr)
        return ret;

    EVP_MD_CTX *ctx = EVP_MD_CTX_create();
    EVP_DigestInit(ctx, alg);
    EVP_DigestUpdate(ctx, data, len);

    uint8_t digest[EVP_MAX_MD_SIZE];
    len = EVP_MAX_MD_SIZE;

    EVP_DigestFinal(ctx, digest, &len);
    hexString(digest, len, ret);

    EVP_MD_CTX_destroy(ctx);
    return ret;
}

bool m::SHA::quick(SHAVersion ver, const uint8_t *data, uint32_t len, uint8_t *result, uint32_t resultSz)
{
    const EVP_MD *alg = g_shaEVP(ver);
    if(alg == nullptr)
        return false;

    if(resultSz < static_cast<uint32_t>(EVP_MD_size(alg)))
        return false;

    EVP_MD_CTX *ctx = EVP_MD_CTX_create();
    EVP_DigestInit(ctx, alg);
    EVP_DigestUpdate(ctx, data, len);
    EVP_DigestFinal(ctx, result, nullptr);
    EVP_MD_CTX_destroy(ctx);
    return true;
}

#endif
