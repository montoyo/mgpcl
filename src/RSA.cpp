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

#include "mgpcl/RSA.h"
#include "mgpcl/INet.h"

#ifndef MGPCL_NO_SSL
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
//Version 1.1.x support
#define M_CRYPTO_11
#endif

#define M_OPENSSL_RSA_VERSION 0
#define m_rsa (*reinterpret_cast<::RSA**>(&m_rsa_))
#define m_crsa static_cast<const ::RSA*>(m_rsa_)

static const int g_paddingMapping[m::kRSAP_Max] = { RSA_NO_PADDING, RSA_PKCS1_PADDING, RSA_PKCS1_OAEP_PADDING, RSA_PKCS1_OAEP_PADDING, RSA_SSLV23_PADDING };
static const int g_algMapping[m::kRSASA_Max] = { NID_sha1, NID_md5, NID_ripemd160, NID_md5_sha1 };

m::RSAPublicKey::RSAPublicKey()
{
}

m::RSAPublicKey::RSAPublicKey(const BigNumber &e, const BigNumber &n) : m_e(e), m_n(n)
{
}

m::RSAPublicKey m::RSAPublicKey::readPEM(InputStream *is, RSAPasswordCallback pc, void *pcud)
{
    BIO *bio = static_cast<BIO*>(inet::makeBIO(is));
    EVP_PKEY *pubKey = nullptr;
    bool ok = (PEM_read_bio_PUBKEY(bio, &pubKey, pc, pcud) != nullptr);
    BIO_free(bio);

    if(!ok)
        throw PEMParseException("PEM_read_bio_PUBKEY failed");

    ::RSA *rsa = EVP_PKEY_get1_RSA(pubKey);
    if(rsa == nullptr) {
        EVP_PKEY_free(pubKey);
        throw PEMParseException("Not an RSA public key");
    }

    RSAPublicKey ret(RSA::fromRaw(rsa).publicKey()); //Do not worry my friends, ref counting is here for ya!
    EVP_PKEY_free(pubKey);
    return ret;
}

m::RSAPublicKey m::RSAPublicKey::readPEM(const m::String &fname, RSAPasswordCallback pc, void *pcud)
{
    BIO *bio = BIO_new_file(fname.raw(), "r");
    EVP_PKEY *pubKey = nullptr;
    bool ok = (PEM_read_bio_PUBKEY(bio, &pubKey, pc, pcud) != nullptr);
    BIO_free(bio);

    if(!ok)
        throw PEMParseException("PEM_read_bio_PUBKEY failed");

    ::RSA *rsa = EVP_PKEY_get1_RSA(pubKey);
    if(rsa == nullptr) {
        EVP_PKEY_free(pubKey);
        throw PEMParseException("Not an RSA public key");
    }

    RSAPublicKey ret(RSA::fromRaw(rsa).publicKey()); //Do not worry my friends, ref counting is here for ya!
    EVP_PKEY_free(pubKey);
    return ret;
}

m::RSAPrivateKey::RSAPrivateKey()
{
}

m::RSAPrivateKey::RSAPrivateKey(const BigNumber &p, const BigNumber &q, const BigNumber &e, const BigNumber &n, const BigNumber &d) : m_e(e), m_n(n), m_d(d), m_p(p), m_q(q)
{
}

m::RSAPrivateKey m::RSAPrivateKey::fromPQE(const BigNumber &p, const BigNumber &q, const BigNumber &e)
{
    BNContext ctx;
    ctx.start();
    BigNumber phi((p - 1ULL).multiply(q - 1ULL, ctx));

    mAssert(e < phi, "e is greater than phi");
    mAssert(e.gcd(phi, ctx).isOne(), "e and phi are not coprimes");

    BigNumber n(p.multiplied(q, ctx));
    BigNumber d(e.modInverse(phi, ctx));
    ctx.end();
    return RSAPrivateKey(p, q, e, n, d);
}

m::RSAPrivateKey m::RSAPrivateKey::fromPQE(const BigNumber &p, const BigNumber &q, RSAExponent e)
{
    uint64_t word;
    switch(e) {
    case kRSAE_3:
        word = 3ULL;
        break;

    case kRSAE_17:
        word = 17ULL;
        break;

    case kRSAE_65537:
        word = 65537ULL;
        break;

    default:
        mAssert(false, "got invalid RSAExponent");
        word = 0; //Make the compiler happy
        break;
    }

    return fromPQE(p, q, BigNumber(word));
}

m::RSAPrivateKey m::RSAPrivateKey::readPEM(InputStream *is, RSAPasswordCallback pc, void *pcud)
{
    BIO *bio = static_cast<BIO*>(inet::makeBIO(is));
    ::RSA *rsa = nullptr;
    bool ok = (PEM_read_bio_RSAPrivateKey(bio, &rsa, pc, pcud) != nullptr);
    BIO_free(bio);

    if(!ok)
        throw PEMParseException("PEM_read_bio_RSAPrivateKey failed");

    return RSA::fromRaw(rsa).privateKey();
}

m::RSAPrivateKey m::RSAPrivateKey::readPEM(const m::String &fname, RSAPasswordCallback pc, void *pcud)
{
    BIO *bio = BIO_new_file(fname.raw(), "r");
    ::RSA *rsa = nullptr;
    bool ok = (PEM_read_bio_RSAPrivateKey(bio, &rsa, pc, pcud) != nullptr);
    BIO_free(bio);

    if(!ok)
        throw PEMParseException("PEM_read_bio_RSAPrivateKey failed");

    return RSA::fromRaw(rsa).privateKey();
}

m::RSAPublicKey m::RSAPrivateKey::publicKey() const
{
    return RSAPublicKey(m_e, m_n);
}

m::RSA::RSA()
{
    m_rsa = RSA_new();
}

m::RSA::~RSA()
{
    if(m_rsa != nullptr)
        RSA_free(m_rsa);
}

bool m::RSA::generateKeys(int bits, RSAExponent exp)
{
    uint64_t expVal;
    switch(exp) {
    case kRSAE_3:
        expVal = 3ULL;
        break;

    case kRSAE_17:
        expVal = 17ULL;
        break;

    case kRSAE_65537:
        expVal = 65537ULL;
        break;

    default:
        return false;
    }

    BIGNUM *expBn = BN_new();
    BN_set_word(expBn, expVal);
    bool ret = (RSA_generate_key_ex(m_rsa, bits, expBn, nullptr) != 0);
    BN_free(expBn);

    return ret;
}

bool m::RSA::publicEncrypt(const uint8_t *src, uint32_t srcLen, uint8_t *dst, RSAPadding padding)
{
    if(padding < 0 || padding >= kRSAP_Max)
        return false;

    return RSA_public_encrypt(static_cast<int>(srcLen), src, dst, m_rsa, g_paddingMapping[padding]) >= 0;
}

int m::RSA::privateDecrypt(const uint8_t *src, uint32_t srcLen, uint8_t *dst, RSAPadding padding)
{
    if(padding < 0 || padding >= kRSAP_Max)
        return false;

    if(padding == kRSAP_PKCS1OAEPSHA256) {
        int num = RSA_size(m_rsa);
        uint8_t *tmp = new uint8_t[num];

        if(RSA_private_decrypt(static_cast<int>(srcLen), src, tmp, m_rsa, RSA_NO_PADDING) <= 0) {
            delete[] tmp;
            return -1;
        }

        int j = 0;
        while(j < num && tmp[j] == 0)
            j++;

        int ret = RSA_padding_check_PKCS1_OAEP_mgf1(dst, num, tmp + j, num - j, num, nullptr, 0, EVP_sha256(), EVP_sha1());
        delete[] tmp;
        return ret;
    } else
        return RSA_private_decrypt(static_cast<int>(srcLen), src, dst, m_rsa, g_paddingMapping[padding]);
}

bool m::RSA::privateEncrypt(const uint8_t *src, uint32_t srcLen, uint8_t *dst, RSAPadding padding)
{
    if(padding != kRSAP_NoPadding && padding != kRSAP_PKCS1)
        return false;

    return RSA_private_encrypt(static_cast<int>(srcLen), src, dst, m_rsa, g_paddingMapping[padding]) >= 0;
}

int m::RSA::publicDecrypt(const uint8_t *src, uint32_t srcLen, uint8_t *dst, RSAPadding padding)
{
    if(padding != kRSAP_NoPadding && padding != kRSAP_PKCS1)
        return false;

    return RSA_public_decrypt(static_cast<int>(srcLen), src, dst, m_rsa, g_paddingMapping[padding]);
}

bool m::RSA::sign(RSASignatureAlgorithm alg, const uint8_t *md, uint32_t mdLen, uint8_t *dst, uint32_t *dstLen)
{
    if(alg < 0 || alg >= kRSASA_Max)
        return false;

    return RSA_sign(g_algMapping[alg], md, mdLen, dst, dstLen, m_rsa) != 0;
}

bool m::RSA::verify(RSASignatureAlgorithm alg, const uint8_t *sigToCheck, uint32_t sigToCheckLen, const uint8_t *md, uint32_t mdLen)
{
    if(alg < 0 || alg >= kRSASA_Max)
        return false;

    return RSA_verify(g_algMapping[alg], md, mdLen, sigToCheck, sigToCheckLen, m_rsa) != 0;
}

uint32_t m::RSA::size() const
{
    return static_cast<uint32_t>(RSA_size(static_cast<const ::RSA*>(m_rsa_)));
}

uint32_t m::RSA::size(RSAPadding padding) const
{
    uint32_t ret = static_cast<uint32_t>(RSA_size(static_cast<const ::RSA*>(m_rsa_)));

    switch(padding) {
    case kRSAP_SSLv23:
    case kRSAP_PKCS1:
        return ret - 12;

    case kRSAP_PKCS1OAEP:
    case kRSAP_PKCS1OAEPSHA256:
        return ret - 42;

    default:
        return ret;
    }
}

#ifdef M_CRYPTO_11

m::BigNumber m::RSA::e() const
{
    const BIGNUM *n;
    const BIGNUM *e;
    const BIGNUM *d;
    RSA_get0_key(m_crsa, &n, &e, &d);
    return e;
}

m::BigNumber m::RSA::n() const
{
    const BIGNUM *n;
    const BIGNUM *e;
    const BIGNUM *d;
    RSA_get0_key(m_crsa, &n, &e, &d);
    return n;
}

m::BigNumber m::RSA::d() const
{
    const BIGNUM *n;
    const BIGNUM *e;
    const BIGNUM *d;
    RSA_get0_key(m_crsa, &n, &e, &d);
    return d;
}

m::BigNumber m::RSA::p() const
{
    const BIGNUM *p;
    const BIGNUM *q;
    RSA_get0_factors(m_crsa, &p, &q);
    return BigNumber(p);
}

m::BigNumber m::RSA::q() const
{
    const BIGNUM *p;
    const BIGNUM *q;
    RSA_get0_factors(m_crsa, &p, &q);
    return BigNumber(q);
}

m::RSAPublicKey m::RSA::publicKey() const
{
    const BIGNUM *n;
    const BIGNUM *e;
    const BIGNUM *d;
    RSA_get0_key(m_crsa, &n, &e, &d);

    return RSAPublicKey(BigNumber(e), BigNumber(n));
}

m::RSAPrivateKey m::RSA::privateKey() const
{
    const BIGNUM *n;
    const BIGNUM *e;
    const BIGNUM *d;
    const BIGNUM *p;
    const BIGNUM *q;

    RSA_get0_key(m_crsa, &n, &e, &d);
    RSA_get0_factors(m_crsa, &p, &q);

    return RSAPrivateKey(BigNumber(p), BigNumber(q), BigNumber(e), BigNumber(n), BigNumber(d));
}

void m::RSA::setPublicKey(const RSAPublicKey &pk)
{
    RSA_set0_key(m_rsa, static_cast<BIGNUM*>(pk.n().rawCopy()), static_cast<BIGNUM*>(pk.e().rawCopy()), nullptr);
}

void m::RSA::setPrivateKey(const RSAPrivateKey &pk)
{
    BIGNUM *p = static_cast<BIGNUM*>(pk.p().rawCopy());
    BIGNUM *q = static_cast<BIGNUM*>(pk.q().rawCopy());
    BIGNUM *d = static_cast<BIGNUM*>(pk.d().rawCopy());

    RSA_set0_key(m_rsa, static_cast<BIGNUM*>(pk.n().rawCopy()), static_cast<BIGNUM*>(pk.e().rawCopy()), d);
    RSA_set0_factors(m_rsa, p, q);

    BIGNUM *dmp1 = BN_new();
    BIGNUM *dmq1 = BN_new();
    BIGNUM *iqmp = BN_new();

    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    BIGNUM *tmp = BN_CTX_get(ctx);

    BN_sub(tmp, p, BN_value_one());
    BN_mod(dmp1, d, tmp, ctx);
    BN_sub(tmp, q, BN_value_one());
    BN_mod(dmq1, d, tmp, ctx);
    BN_mod_inverse(iqmp, q, p, ctx);

    BN_CTX_end(ctx);
    BN_CTX_free(ctx);

    RSA_set0_crt_params(m_rsa, dmp1, dmq1, iqmp);
}

#else

m::BigNumber m::RSA::e() const
{
    return BigNumber(m_crsa->e);
}

m::BigNumber m::RSA::n() const
{
    return BigNumber(m_crsa->n);
}

m::BigNumber m::RSA::d() const
{
    return BigNumber(m_crsa->d);
}

m::BigNumber m::RSA::p() const
{
    return BigNumber(m_crsa->p);
}

m::BigNumber m::RSA::q() const
{
    return BigNumber(m_crsa->q);
}

m::RSAPublicKey m::RSA::publicKey() const
{
    return RSAPublicKey(BigNumber(m_crsa->e), BigNumber(m_crsa->n));
}

m::RSAPrivateKey m::RSA::privateKey() const
{
    return RSAPrivateKey(BigNumber(m_crsa->p), BigNumber(m_crsa->q), BigNumber(m_crsa->e), BigNumber(m_crsa->n), BigNumber(m_crsa->d));
}

void m::RSA::setPublicKey(const RSAPublicKey &pk)
{
    if(m_rsa->e != nullptr)
        BN_free(m_rsa->e);

    if(m_rsa->n != nullptr)
        BN_free(m_rsa->n);

    m_rsa->e = static_cast<BIGNUM*>(pk.e().rawCopy());
    m_rsa->n = static_cast<BIGNUM*>(pk.n().rawCopy());
}

void m::RSA::setPrivateKey(const RSAPrivateKey &pk)
{
    if(m_rsa->p != nullptr)
        BN_clear_free(m_rsa->p);

    if(m_rsa->q != nullptr)
        BN_clear_free(m_rsa->q);

    if(m_rsa->e != nullptr)
        BN_free(m_rsa->e);

    if(m_rsa->n != nullptr)
        BN_free(m_rsa->n);

    if(m_rsa->d != nullptr)
        BN_clear_free(m_rsa->d);

    if(m_rsa->dmp1 != nullptr)
        BN_clear_free(m_rsa->dmp1);

    if(m_rsa->dmq1 != nullptr)
        BN_clear_free(m_rsa->dmq1);

    if(m_rsa->iqmp != nullptr)
        BN_clear_free(m_rsa->iqmp);

    m_rsa->p = static_cast<BIGNUM*>(pk.p().rawCopy());
    m_rsa->q = static_cast<BIGNUM*>(pk.q().rawCopy());
    m_rsa->e = static_cast<BIGNUM*>(pk.e().rawCopy());
    m_rsa->n = static_cast<BIGNUM*>(pk.n().rawCopy());
    m_rsa->d = static_cast<BIGNUM*>(pk.d().rawCopy());

    m_rsa->dmp1 = BN_new(); //Not sure I need to re-allocate these...
    m_rsa->dmq1 = BN_new();
    m_rsa->iqmp = BN_new();

    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    BIGNUM *tmp = BN_CTX_get(ctx);

    BN_sub(tmp, m_rsa->p, BN_value_one());
    BN_mod(m_rsa->dmp1, m_rsa->d, tmp, ctx);
    BN_sub(tmp, m_rsa->q, BN_value_one());
    BN_mod(m_rsa->dmq1, m_rsa->d, tmp, ctx);
    BN_mod_inverse(m_rsa->iqmp, m_rsa->q, m_rsa->p, ctx);

    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    m_rsa->version = M_OPENSSL_RSA_VERSION;
}

#endif
#endif
