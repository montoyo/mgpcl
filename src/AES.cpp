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

#include "mgpcl/AES.h"

#ifndef MGPCL_NO_SSL
#include <openssl/evp.h>

typedef const EVP_CIPHER*(*AESFunc)();
static AESFunc g_mapping[m::kAESV_Max] = { nullptr, EVP_aes_128_cbc, EVP_aes_192_cbc, EVP_aes_256_cbc };

#define m_aes (*reinterpret_cast<EVP_CIPHER_CTX**>(&m_aes_))
#define m_caes static_cast<EVP_CIPHER_CTX*>(m_aes_)
#define AES_OF(obj) static_cast<EVP_CIPHER_CTX*>((obj).m_aes_)

m::AES::AES()
{
    m_mode = kAESM_None;
    m_version = kAESV_None;
    m_aes = EVP_CIPHER_CTX_new();
}

m::AES::AES(AESMode mode, AESVersion version, const uint8_t *key, const uint8_t *iv)
{
    m_aes = EVP_CIPHER_CTX_new(); //Always create a cipher context

    if((mode == kAESM_Encrypt || mode == kAESM_Decrypt) && (version > kAESV_None && version < kAESV_Max)) {
        if(EVP_CipherInit(m_aes, g_mapping[version](), key, iv, mode == kAESM_Encrypt) != 0) {
            m_mode = mode;
            m_version = version;
            return;
        }
    }

    m_mode = kAESM_None;
    m_version = kAESV_None;
}

m::AES::AES(const AES &src)
{
    m_mode = src.m_mode;
    m_version = src.m_version;
    m_aes = EVP_CIPHER_CTX_new();

    if(m_mode != kAESM_None)
        EVP_CIPHER_CTX_copy(m_aes, AES_OF(src));
}

m::AES::AES(AES &&src) noexcept
{
    m_mode = src.m_mode;
    m_version = src.m_version;
    m_aes = AES_OF(src);
    src.m_aes_ = nullptr;
}

m::AES::~AES()
{
    if(m_aes != nullptr)
        EVP_CIPHER_CTX_free(m_aes);
}

bool m::AES::init(AESMode mode, AESVersion version, const uint8_t *key, const uint8_t *iv)
{
    if(mode != kAESM_Encrypt && mode != kAESM_Decrypt)
        return false;

    if(version <= kAESV_None || version >= kAESV_Max)
        return false;

    if(EVP_CipherInit(m_aes, g_mapping[version](), key, iv, mode == kAESM_Encrypt) == 0)
        return false;

    m_mode = mode;
    m_version = version;
    return true;
}

int m::AES::update(const uint8_t *src, uint32_t srcLen, uint8_t *dst, uint32_t dstLen)
{
    if(m_mode == kAESM_None)
        return -1;

    int idst = static_cast<int>(dstLen);
    if(EVP_CipherUpdate(m_aes, dst, &idst, src, static_cast<int>(srcLen)) == 0)
        return -1;

    return idst;
}

bool m::AES::update(const uint8_t *src, uint32_t srcLen, uint8_t *dst, uint32_t *dstLen)
{
    if(m_mode == kAESM_None)
        return false;

    int idst = static_cast<int>(*dstLen);
    if(EVP_CipherUpdate(m_aes, dst, &idst, src, static_cast<int>(srcLen)) == 0)
        return false;

    *dstLen = static_cast<uint32_t>(idst);
    return true;
}

int m::AES::final(uint8_t *dst, uint32_t dstLen)
{
    if(m_mode == kAESM_None)
        return false;

    int idst = static_cast<int>(dstLen);
    if(EVP_CipherFinal(m_aes, dst, &idst) == 0)
        return -1;

    return idst;
}

bool m::AES::final(uint8_t *dst, uint32_t *dstLen)
{
    if(m_mode == kAESM_None)
        return false;

    int idst = static_cast<int>(*dstLen);
    if(EVP_CipherFinal(m_aes, dst, &idst) == 0)
        return false;

    *dstLen = static_cast<uint32_t>(idst);
    return true;
}

void m::AES::reset(const uint8_t *key, const uint8_t *iv)
{
    if(m_mode != kAESM_None) {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
        EVP_CIPHER_CTX_reset(m_aes);
#else
		EVP_CIPHER_CTX_free(m_aes);
		m_aes = EVP_CIPHER_CTX_new();
#endif

        EVP_CipherInit(m_aes, g_mapping[m_version](), key, iv, m_mode == kAESM_Encrypt);
    }
}

m::AES &m::AES::operator = (const AES &src)
{
    if(m_aes_ == src.m_aes_)
        return *this;

    if(m_mode != kAESM_None) {
        EVP_CIPHER_CTX_free(m_aes);
        m_aes = EVP_CIPHER_CTX_new();
    }

    m_mode = src.m_mode;
    m_version = src.m_version;

    if(m_mode != kAESM_None)
        EVP_CIPHER_CTX_copy(m_aes, AES_OF(src));

    return *this;
}

m::AES &m::AES::operator = (AES &&src) noexcept
{
    EVP_CIPHER_CTX_free(m_aes);

    m_mode = src.m_mode;
    m_version = src.m_version;
    m_aes = AES_OF(src);
    src.m_aes_ = nullptr;

    return *this;
}

uint32_t m::AES::blockSize() const
{
    if(m_mode == kAESM_None)
        return 0;

    return static_cast<uint32_t>(EVP_CIPHER_CTX_block_size(m_caes));
}

uint32_t m::AES::keySize() const
{
    if(m_mode == kAESM_None)
        return 0;

    return static_cast<uint32_t>(EVP_CIPHER_CTX_key_length(m_caes));
}

uint32_t m::AES::ivSize() const
{
    if(m_mode == kAESM_None)
        return 0;

    return static_cast<uint32_t>(EVP_CIPHER_CTX_iv_length(m_caes));
}

uint32_t m::AES::blockSize(AESVersion ver)
{
    if(ver <= kAESV_None || ver >= kAESV_Max)
        return 0;

    return static_cast<uint32_t>(EVP_CIPHER_block_size(g_mapping[ver]()));
}

uint32_t m::AES::keySize(AESVersion ver)
{
    if(ver <= kAESV_None || ver >= kAESV_Max)
        return 0;

    return static_cast<uint32_t>(EVP_CIPHER_key_length(g_mapping[ver]()));
}

uint32_t m::AES::ivSize(AESVersion ver)
{
    if(ver <= kAESV_None || ver >= kAESV_Max)
        return 0;

    return static_cast<uint32_t>(EVP_CIPHER_iv_length(g_mapping[ver]()));
}

#endif
