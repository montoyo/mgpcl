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

#include "mgpcl/HMAC.h"

#ifndef MGPCL_NO_SSL
#include <openssl/hmac.h>

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
//Version 1.1.x support
#define M_CRYPTO_11
#endif

#define m_ctx (*reinterpret_cast<HMAC_CTX**>(&m_ctx_))

static const EVP_MD *g_hashMD(m::HMACHash hash)
{
    switch(hash) {
    case m::kHH_Sha1:
        return EVP_sha1();

    case m::kHH_Sha224:
        return EVP_sha224();

    case m::kHH_Sha256:
        return EVP_sha256();

    case m::kHH_Sha384:
        return EVP_sha384();

    case m::kHH_Sha512:
        return EVP_sha512();

    case m::kHH_Ripemd160:
        return EVP_ripemd160();

    default:
        return nullptr;
    }
}

m::HMAC::HMAC()
{
    m_hash = kHH_None;
    m_ctx = nullptr;
}

m::HMAC::HMAC(HMACHash hash, const uint8_t *key, int keyLen)
{
    const EVP_MD *alg = g_hashMD(hash);

    if(alg == nullptr) {
        m_hash = kHH_None;
        m_ctx = nullptr;
    } else {
        m_hash = hash;

#ifdef M_CRYPTO_11
        m_ctx = HMAC_CTX_new();
#else
        m_ctx = new HMAC_CTX;
        HMAC_CTX_init(m_ctx);
#endif

        HMAC_Init_ex(m_ctx, key, keyLen, alg, nullptr);
    }
}

m::HMAC::HMAC(const HMAC &src)
{
    if(src.m_ctx_ == nullptr) {
        m_hash = kHH_None;
        m_ctx = nullptr;
    } else {
        m_hash = src.m_hash;

#ifdef M_CRYPTO_11
        m_ctx = HMAC_CTX_new();
#else
        m_ctx = new HMAC_CTX;
        HMAC_CTX_init(m_ctx);
#endif

        HMAC_CTX_copy(m_ctx, static_cast<HMAC_CTX*>(src.m_ctx_));
    }
}

m::HMAC::HMAC(HMAC &&src)
{
    m_hash = src.m_hash;
    m_ctx_ = src.m_ctx_;
    src.m_ctx_ = nullptr;
}

m::HMAC::~HMAC()
{
    if(m_ctx != nullptr) {
#ifdef M_CRYPTO_11
        HMAC_CTX_free(m_ctx);
#else
        HMAC_CTX_cleanup(m_ctx);
        delete m_ctx;
#endif
    }
}

bool m::HMAC::init(HMACHash hash, const uint8_t *key, int keyLen)
{
    const EVP_MD *alg = g_hashMD(hash);
    if(alg == nullptr) {
        if(m_ctx != nullptr) {
#ifdef M_CRYPTO_11
            HMAC_CTX_free(m_ctx);
#else
            HMAC_CTX_cleanup(m_ctx);
            delete m_ctx;
#endif

            m_ctx = nullptr;
        }

        m_hash = kHH_None;
        m_ctx = nullptr;
        return false;
    } else {
        m_hash = hash;

#ifdef M_CRYPTO_11
        if(m_ctx == nullptr)
            m_ctx = HMAC_CTX_new();
#else
        if(m_ctx == nullptr)
            m_ctx = new HMAC_CTX;
        else
            HMAC_CTX_cleanup(m_ctx); //Is this really needed?

        HMAC_CTX_init(m_ctx);
#endif

        HMAC_Init_ex(m_ctx, key, keyLen, alg, nullptr);
        return true;
    }
}

void m::HMAC::reset()
{
#ifdef M_CRYPTO_11
    if(m_ctx != nullptr)
        HMAC_Init_ex(m_ctx, nullptr, 0, HMAC_CTX_get_md(m_ctx), nullptr);
#else
    if(m_ctx != nullptr)
        HMAC_Init_ex(m_ctx, nullptr, 0, m_ctx->md, nullptr);
#endif
}

void m::HMAC::update(const uint8_t *src, uint32_t sz)
{
    mDebugAssert(m_ctx != nullptr, "forgot to init HMAC");
    mDebugAssert(src != nullptr, "passed nullptr source buffer");

    HMAC_Update(m_ctx, src, static_cast<size_t>(sz));
}

void m::HMAC::update(const char *str, int len)
{
    mDebugAssert(m_ctx != nullptr, "forgot to init HMAC");
    mDebugAssert(str != nullptr, "passed nullptr source string");

    if(len < 0) {
        len = 0;

        while(str[len] != 0)
            len++;
    }

    HMAC_Update(m_ctx, reinterpret_cast<const uint8_t*>(str), static_cast<size_t>(len));
}

void m::HMAC::update(const String &str)
{
    mDebugAssert(m_ctx != nullptr, "forgot to init HMAC");
    HMAC_Update(m_ctx, reinterpret_cast<const uint8_t*>(str.raw()), static_cast<size_t>(str.length()));
}

bool m::HMAC::final(uint8_t *dst, uint32_t sz)
{
    mDebugAssert(m_ctx != nullptr, "forgot to init HMAC");
    mDebugAssert(dst != nullptr, "passed nullptr destination buffer");

    if(sz < static_cast<uint32_t>(HMAC_size(m_ctx)))
        return false;

    HMAC_Final(m_ctx, dst, nullptr);
    return true;
}

void m::HMAC::final(String &dst)
{
    mDebugAssert(m_ctx != nullptr, "forgot to init HMAC");

    uint8_t buf[EVP_MAX_MD_SIZE];
    uint32_t sz = EVP_MAX_MD_SIZE;

    HMAC_Final(m_ctx, buf, &sz);
    hexString(buf, sz, dst);
}

uint8_t *m::HMAC::final()
{
    mDebugAssert(m_ctx != nullptr, "forgot to init HMAC");

    uint8_t *ret = new uint8_t[HMAC_size(m_ctx)];
    HMAC_Final(m_ctx, ret, nullptr);
    return ret;
}

uint32_t m::HMAC::hashSize() const
{
    if(m_ctx_ == nullptr)
        return 0;

    return static_cast<uint32_t>(HMAC_size(static_cast<const HMAC_CTX*>(m_ctx_)));
}

uint32_t m::HMAC::hashSize(HMACHash ver)
{
    const EVP_MD *alg = g_hashMD(ver);
    if(alg == nullptr)
        return 0;

    return static_cast<uint32_t>(EVP_MD_size(alg));
}

m::HMAC &m::HMAC::operator = (const HMAC &src)
{
    if(src.m_ctx_ == nullptr) {
        if(m_ctx != nullptr) {
#ifdef M_CRYPTO_11
            HMAC_CTX_free(m_ctx);
#else
            HMAC_CTX_cleanup(m_ctx);
            delete m_ctx;
#endif
        }

        m_hash = kHH_None;
        m_ctx = nullptr;
    } else {
#ifdef M_CRYPTO_11
        if(m_ctx == nullptr)
            m_ctx = HMAC_CTX_new();
#else
        if(m_ctx == nullptr)
            m_ctx = new HMAC_CTX;
        else
            HMAC_CTX_cleanup(m_ctx);

        HMAC_CTX_init(m_ctx);
#endif

        m_hash = src.m_hash;
        HMAC_CTX_copy(m_ctx, static_cast<HMAC_CTX*>(src.m_ctx_));
    }

    return *this;
}

m::HMAC &m::HMAC::operator = (HMAC &&src)
{
    if(m_ctx != nullptr) {
#ifdef M_CRYPTO_11
        HMAC_CTX_free(m_ctx);
#else
        HMAC_CTX_cleanup(m_ctx);
        delete m_ctx;
#endif
    }

    m_hash = src.m_hash;
    m_ctx_ = src.m_ctx_;
    src.m_ctx_ = nullptr;
    return *this;
}

bool m::HMAC::quick(HMACHash hash, const uint8_t *key, int keyLen, const uint8_t *data, uint32_t dataLen, uint8_t *result, uint32_t resultSz)
{
    const EVP_MD *alg = g_hashMD(hash);
    if(alg == nullptr)
        return false;

    if(resultSz < static_cast<uint32_t>(EVP_MD_size(alg)))
        return false;

#ifdef M_CRYPTO_11
    HMAC_CTX *ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, key, keyLen, alg, nullptr);
    HMAC_Update(ctx, data, static_cast<size_t>(dataLen));
    HMAC_Final(ctx, result, nullptr);
    HMAC_CTX_free(ctx);
#else
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key, keyLen, alg, nullptr);
    HMAC_Update(&ctx, data, static_cast<size_t>(dataLen));
    HMAC_Final(&ctx, result, nullptr);
    HMAC_CTX_cleanup(&ctx);
#endif

    return true;
}

m::String m::HMAC::quick(HMACHash hash, const uint8_t *key, int keyLen, const uint8_t *data, uint32_t dataLen)
{
    String ret;
    const EVP_MD *alg = g_hashMD(hash);
    if(alg == nullptr)
        return ret;

    uint8_t result[EVP_MAX_MD_SIZE];
    uint32_t len = EVP_MAX_MD_SIZE;

#ifdef M_CRYPTO_11
    HMAC_CTX *ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, key, keyLen, alg, nullptr);
    HMAC_Update(ctx, data, static_cast<size_t>(dataLen));
    HMAC_Final(ctx, result, &len);
    HMAC_CTX_free(ctx);
#else
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key, keyLen, alg, nullptr);
    HMAC_Update(&ctx, data, static_cast<size_t>(dataLen));
    HMAC_Final(&ctx, result, &len);
    HMAC_CTX_cleanup(&ctx);
#endif

    hexString(result, len, ret);
    return len;
}

#endif
