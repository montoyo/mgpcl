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
#include "BigNumber.h"
#include "Config.h"
#include "IOStream.h"
#include <exception>

#ifndef MGPCL_NO_SSL
#define M_RSA_SIZE(bits) ((bits) / 8)
#define M_RSA_PKCS1_SIZE(bits) ((bits) / 8 - 12)
#define M_RSA_PKCS1OAEP_SIZE(bits) ((bits) / 8 - 42)
#define M_RSA_SSLv23_SIZE(bits) M_RSA_PKCS1_SIZE(bits)

namespace m
{
    typedef int (*RSAPasswordCallback)(char *buf, int size, int rwflag, void *userdata);

    enum RSAExponent
    {
        kRSAE_3,
        kRSAE_17,
        kRSAE_65537
    };

    enum RSAPadding
    {
        kRSAP_NoPadding = 0, //Insecure. Do not use unless you do manual padding.
        kRSAP_PKCS1,
        kRSAP_PKCS1OAEP,
        kRSAP_PKCS1OAEPSHA256, //Private decrypt only. If used somewhere else, will fall back to regular PCKS1 OAEP (SHA-1) padding.
        kRSAP_SSLv23,

        kRSAP_Max //NOT AN ACTUAL PADDING. KEEP AT END.
    };

    enum RSASignatureAlgorithm
    {
        kRSASA_SHA1 = 0,
        kRSASA_MD5,
        kRSASA_RipeMD160,
        kRSASA_MD5SHA1,

        kRSASA_Max //DO NOT USE. KEEP AT END.
    };

    class PEMParseException : public std::exception
    {
    public:
        PEMParseException(const char *msg) noexcept : m_msg(msg)
        {
        }

        const char *what() const noexcept override
        {
            return m_msg;
        }

    private:
        const char *m_msg;
    };

    class RSAPublicKey
    {
    public:
        RSAPublicKey();
        RSAPublicKey(const BigNumber &e, const BigNumber &n);

        static RSAPublicKey readPEM(InputStream *is, RSAPasswordCallback pc = nullptr, void *pcud = nullptr); //Throws PEMParseException
        static RSAPublicKey readPEM(const String &fname, RSAPasswordCallback pc = nullptr, void *pcud = nullptr); //Throws PEMParseException

        const BigNumber &e() const
        {
            return m_e;
        }

        const BigNumber &n() const
        {
            return m_n;
        }

        BigNumber &e()
        {
            return m_e;
        }

        BigNumber &n()
        {
            return m_n;
        }

    private:
        BigNumber m_e;
        BigNumber m_n;
    };

    class RSAPrivateKey
    {
    public:
        RSAPrivateKey();
        RSAPrivateKey(const BigNumber &p, const BigNumber &q, const BigNumber &e, const BigNumber &n, const BigNumber &d);

        static RSAPrivateKey fromPQE(const BigNumber &p, const BigNumber &q, const BigNumber &e);
        static RSAPrivateKey fromPQE(const BigNumber &p, const BigNumber &q, RSAExponent e);
        static RSAPrivateKey readPEM(InputStream *is, RSAPasswordCallback pc = nullptr, void *pcud = nullptr); //Throws PEMParseException
        static RSAPrivateKey readPEM(const String &fname, RSAPasswordCallback pc = nullptr, void *pcud = nullptr); //Throws PEMParseException

        RSAPublicKey publicKey() const;

        const BigNumber &e() const
        {
            return m_e;
        }

        const BigNumber &n() const
        {
            return m_n;
        }

        const BigNumber &d() const
        {
            return m_d;
        }

        const BigNumber &p() const
        {
            return m_p;
        }

        const BigNumber &q() const
        {
            return m_q;
        }

        BigNumber &e()
        {
            return m_e;
        }

        BigNumber &n()
        {
            return m_n;
        }

        BigNumber &d()
        {
            return m_d;
        }

        BigNumber &p()
        {
            return m_p;
        }

        BigNumber &q()
        {
            return m_q;
        }

    private:
        BigNumber m_e;
        BigNumber m_n;
        BigNumber m_d;
        BigNumber m_p;
        BigNumber m_q;
    };

    class RSA
    {
    public:
        RSA();
        ~RSA();

        static RSA fromRaw(void *raw)
        {
            return RSA(raw);
        }

        bool generateKeys(int bits, RSAExponent exp = kRSAE_65537);
        BigNumber e() const; //Public exponent
        BigNumber n() const; //Modulus
        BigNumber d() const; //Private exponent
        BigNumber p() const;
        BigNumber q() const;

        RSAPublicKey publicKey() const;
        RSAPrivateKey privateKey() const;
        void setPublicKey(const RSAPublicKey &pk);
        void setPrivateKey(const RSAPrivateKey &pk);

        /* If no padding is used, srcLen must be exactly this->size().
         * Otherwise, srcLen must less or equal to this->size(padding).
         * Output buffer will contain this->size() bytes, if everything
         * went well, i.e., publicEncrypt() returned true.
         */
        bool publicEncrypt(const uint8_t *src, uint32_t srcLen, uint8_t *dst, RSAPadding padding);

        /* dst must be able to handle this->size() bytes.
         * If deciphering went well, privateDecrypt() returns
         * the number of bytes deciphered. If something failed,
         * privateDecrypt() will return a negative number.
         */
        int privateDecrypt(const uint8_t *src, uint32_t srcLen, uint8_t *dst, RSAPadding padding);

        /* No information about srcLen given by the OpenSSL docs,
         * but I guess it must be less or equal to this->size(padding).
         * Note that only the kRSAP_PKCS1 padding will work here.
         * 
         * dst must be able to handle this->size() bytes. If it worked,
         * privateEncrypt() will return true, and false otherwise.
         */
        bool privateEncrypt(const uint8_t *src, uint32_t srcLen, uint8_t *dst, RSAPadding padding);

        /* dst must be able to handle this->size() bytes.
         * According to OpenSSL's docs, the destination size
         * will not exceed this->size() - 11. Also, just like
         * with privateEncrypt(), only kRSAP_PKCS1 padding is
         * supported.
         * 
         * Returns the size of the deciphered message, or -1 on error.
         */
        int publicDecrypt(const uint8_t *src, uint32_t srcLen, uint8_t *dst, RSAPadding padding);

        /* alg specifies the algorithm used to generate the message digest md.
         * Nothing is said about mdLen in the OpenSSL docs.
         * dst will contain at most this->size() bytes.
         * dstLen is write-only and will contain the size of dst.
         *
         * Returns true on successful signing.
         */
        bool sign(RSASignatureAlgorithm alg, const uint8_t *md, uint32_t mdLen, uint8_t *dst, uint32_t *dstLen);

        /* alg specifies the algorithm used to generate the message digest md.
         * Returns true upon successful verification.
         */
        bool verify(RSASignatureAlgorithm alg, const uint8_t *sigToCheck, uint32_t sigToCheckLen, const uint8_t *md, uint32_t mdLen);

        uint32_t size() const;
        uint32_t size(RSAPadding padding) const;

    private:
        RSA(void *opensslRSAStruct)
        {
            m_rsa_ = opensslRSAStruct;
        }

        void *m_rsa_;
    };
}

#endif
