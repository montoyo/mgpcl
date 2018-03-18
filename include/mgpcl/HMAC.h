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
#include "String.h"
#include "Config.h"

#ifndef MGPCL_NO_SSL

namespace m
{
    enum HMACHash
    {
        kHH_None = 0,
        kHH_Sha1,
        kHH_Sha224,
        kHH_Sha256,
        kHH_Sha384,
        kHH_Sha512,
        kHH_Ripemd160
    };

    class HMAC
    {
    public:
        HMAC();
        HMAC(HMACHash hash, const uint8_t *key, int keyLen);
        HMAC(const HMAC &src);
        HMAC(HMAC &&src);
        ~HMAC();

        bool init(HMACHash hash, const uint8_t *key, int keyLen);
        void reset();
        void update(const uint8_t *src, uint32_t sz);
        void update(const char *str, int len = -1);
        void update(const String &str);
        bool final(uint8_t *dst, uint32_t sz);
        void final(String &dst); //Appends the hex string corresponding to the digest to 'dst'.
        uint8_t *final(); //Free it using delete[]

        static bool quick(HMACHash hash, const uint8_t *key, int keyLen, const uint8_t *data, uint32_t dataLen, uint8_t *result, uint32_t resultSz);
        static String quick(HMACHash hash, const uint8_t *key, int keyLen, const uint8_t *data, uint32_t dataLen);

        static bool quick(HMACHash hash, const uint8_t *key, int keyLen, const String &data, uint8_t *result, uint32_t resultSz)
        {
            return quick(hash, key, keyLen, reinterpret_cast<const uint8_t*>(data.raw()), static_cast<uint32_t>(data.length()), result, resultSz);
        }

        static String quick(HMACHash hash, const uint8_t *key, int keyLen, const String &data)
        {
            return quick(hash, key, keyLen, reinterpret_cast<const uint8_t*>(data.raw()), static_cast<uint32_t>(data.length()));
        }

        uint32_t hashSize() const;
        static uint32_t hashSize(HMACHash ver);

        HMACHash hash() const
        {
            return m_hash;
        }

        bool isValid() const
        {
            return m_hash != kHH_None && m_ctx_ != nullptr;
        }

        HMAC &operator = (const HMAC &src);
        HMAC &operator = (HMAC &&src);

    private:
        HMACHash m_hash;
        void *m_ctx_;
    };
}

#endif
