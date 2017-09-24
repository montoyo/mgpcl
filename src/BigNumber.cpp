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

#include "mgpcl/BigNumber.h"

#ifndef MGPCL_NO_SSL
#include <openssl/bn.h>

#define m_ctx (*reinterpret_cast<BN_CTX**>(&m_ctx_))
#define CTX_OF(x) static_cast<BN_CTX*>((x).m_ctx_)

m::BNContext::BNContext()
{
    m_ctx = BN_CTX_new();
    m_refs = new int(1);
}

m::BNContext::BNContext(const BNContext &src)
{
    m_ctx_ = src.m_ctx_;
    m_refs = src.m_refs;
    (*m_refs)++;
}

m::BNContext::BNContext(BNContext &&src)
{
    m_ctx_ = src.m_ctx_;
    m_refs = src.m_refs;
    src.m_refs = nullptr;
}

m::BNContext::~BNContext()
{
    if(m_refs != nullptr && --(*m_refs) <= 0) {
        BN_CTX_free(m_ctx);
        delete m_refs;
    }
}

m::BNContext &m::BNContext::operator = (const BNContext &src)
{
    if(m_ctx_ == src.m_ctx_)
        return *this;

    if(--(*m_refs) <= 0) {
        BN_CTX_free(m_ctx);
        delete m_refs;
    }

    m_ctx_ = src.m_ctx_;
    m_refs = src.m_refs;
    (*m_refs)++;
    return *this;
}

m::BNContext &m::BNContext::operator = (BNContext &&src)
{
    if(--(*m_refs) <= 0) {
        BN_CTX_free(m_ctx);
        delete m_refs;
    }

    m_ctx_ = src.m_ctx_;
    m_refs = src.m_refs;
    src.m_refs = nullptr;
    return *this;
}

void m::BNContext::start()
{
    BN_CTX_start(m_ctx);
}

void m::BNContext::end()
{
    BN_CTX_end(m_ctx);
}

#define m_bn (*reinterpret_cast<BIGNUM**>(&m_bn_))
#define m_cbn static_cast<const BIGNUM*>(m_bn_)
#define BN_OF(x) static_cast<BIGNUM*>((x).m_bn_)
#define CBN_OF(x) static_cast<const BIGNUM*>((x).m_bn_)

m::BigNumber::BigNumber()
{
    m_bn = BN_new();
}

m::BigNumber::BigNumber(uint64_t word)
{
    m_bn = BN_new();
    BN_set_word(m_bn, word);
}

m::BigNumber::BigNumber(const void *raw)
{
    if(raw == nullptr)
        m_bn = BN_new();
    else
        m_bn = BN_dup(static_cast<const BIGNUM*>(raw));
}

m::BigNumber::BigNumber(const uint8_t *data, uint32_t len)
{
    m_bn = BN_bin2bn(data, static_cast<int>(len), nullptr);
}

m::BigNumber::BigNumber(const String &data, bool isHex)
{
    if(isHex)
        BN_hex2bn(&m_bn, data.raw());
    else
        BN_dec2bn(&m_bn, data.raw());
}

m::BigNumber::BigNumber(const BigNumber &src)
{
    m_bn = BN_dup(CBN_OF(src));
}

m::BigNumber::BigNumber(BigNumber &&src)
{
    m_bn = BN_OF(src);
    src.m_bn_ = nullptr;
}

m::BigNumber::~BigNumber()
{
    if(m_bn != nullptr)
        BN_clear_free(m_bn);
}

m::BigNumber &m::BigNumber::zero()
{
    BN_zero(m_bn);
    return *this;
}

m::BigNumber &m::BigNumber::setWord(uint64_t word)
{
    BN_set_word(m_bn, word);
    return *this;
}

m::BigNumber m::BigNumber::operator + (const BigNumber &src) const
{
    BigNumber ret;
    BN_add(BN_OF(ret), m_cbn, CBN_OF(src));
    return ret;
}

m::BigNumber m::BigNumber::operator - (const BigNumber &src) const
{
    BigNumber ret;
    BN_sub(BN_OF(ret), m_cbn, CBN_OF(src));
    return ret;
}

m::BigNumber m::BigNumber::operator * (const BigNumber &src) const
{
    BN_CTX *ctx = BN_CTX_new(); //Slow, ikr.
    BN_CTX_start(ctx);
    BigNumber ret;
    BN_mul(BN_OF(ret), m_cbn, CBN_OF(src), ctx);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    return ret;
}

m::BigNumber m::BigNumber::operator / (const BigNumber &src) const
{
    BN_CTX *ctx = BN_CTX_new(); //Slow, ikr.
    BN_CTX_start(ctx);
    BigNumber ret;
    BN_div(BN_OF(ret), nullptr, m_cbn, CBN_OF(src), ctx);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    return ret;
}

m::BigNumber m::BigNumber::operator % (const BigNumber &src) const
{
    BN_CTX *ctx = BN_CTX_new(); //Slow, ikr.
    BN_CTX_start(ctx);
    BigNumber ret;
    BN_mod(BN_OF(ret), m_cbn, CBN_OF(src), ctx);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);

    return ret;
}

m::BigNumber m::BigNumber::operator << (int t) const
{
    BigNumber ret;
    BN_lshift(BN_OF(ret), m_cbn, t);

    return ret;
}

m::BigNumber m::BigNumber::operator >> (int t) const
{
    BigNumber ret;
    BN_rshift(BN_OF(ret), m_cbn, t);

    return ret;
}

m::BigNumber m::BigNumber::operator + (uint64_t word) const
{
    BigNumber ret(*this);
    BN_add_word(BN_OF(ret), word);

    return ret;
}

m::BigNumber m::BigNumber::operator - (uint64_t word) const
{
    BigNumber ret(*this);
    BN_sub_word(BN_OF(ret), word);

    return ret;
}

m::BigNumber m::BigNumber::operator * (uint64_t word) const
{
    BigNumber ret(*this);
    BN_mul_word(BN_OF(ret), word);

    return ret;
}

m::BigNumber m::BigNumber::operator / (uint64_t word) const
{
    BigNumber ret(*this);
    BN_div_word(BN_OF(ret), word);

    return ret;
}

m::BigNumber m::BigNumber::operator % (uint64_t word) const
{
    BigNumber ret(*this);
    BN_mod_word(BN_OF(ret), word);

    return ret;
}

m::BigNumber &m::BigNumber::operator += (const BigNumber &src)
{
    BN_add(m_bn, m_cbn, CBN_OF(src));
    return *this;
}

m::BigNumber &m::BigNumber::operator -= (const BigNumber &src)
{
    BN_sub(m_bn, m_cbn, CBN_OF(src)); //Can I do that?
    return *this;
}

m::BigNumber &m::BigNumber::operator *= (const BigNumber &src)
{
    BN_CTX *ctx = BN_CTX_new(); //Slow, ikr.
    BN_CTX_start(ctx);
    BN_mul(m_bn, m_cbn, CBN_OF(src), ctx);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    return *this;
}

m::BigNumber &m::BigNumber::operator /= (const BigNumber &src)
{
    BN_CTX *ctx = BN_CTX_new(); //Slow, ikr.
    BN_CTX_start(ctx);
    BN_div(m_bn, nullptr, m_cbn, CBN_OF(src), ctx);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    return *this;
}

m::BigNumber &m::BigNumber::operator %= (const BigNumber &src)
{
    BN_CTX *ctx = BN_CTX_new(); //Slow, ikr.
    BN_CTX_start(ctx);
    BN_mod(m_bn, m_cbn, CBN_OF(src), ctx);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    return *this;
}

m::BigNumber &m::BigNumber::operator <<= (int t)
{
    BN_lshift(m_bn, m_cbn, t);
    return *this;
}

m::BigNumber &m::BigNumber::operator >>= (int t)
{
    BN_rshift(m_bn, m_cbn, t);
    return *this;
}

m::BigNumber &m::BigNumber::operator += (uint64_t word)
{
    BN_add_word(m_bn, word);
    return *this;
}

m::BigNumber &m::BigNumber::operator -= (uint64_t word)
{
    BN_sub_word(m_bn, word);
    return *this;
}

m::BigNumber &m::BigNumber::operator *= (uint64_t word)
{
    BN_mul_word(m_bn, word);
    return *this;
}

m::BigNumber &m::BigNumber::operator /= (uint64_t word)
{
    BN_div_word(m_bn, word);
    return *this;
}

m::BigNumber &m::BigNumber::operator %= (uint64_t word)
{
    BN_mod_word(m_bn, word);
    return *this;
}

m::BigNumber &m::BigNumber::multiply(const BigNumber &src, BNContext &ctx)
{
    BN_mul(m_bn, m_cbn, CBN_OF(src), CTX_OF(ctx));
    return *this;
}

m::BigNumber &m::BigNumber::divide(const BigNumber &src, BNContext &ctx)
{
    BN_div(m_bn, nullptr, m_cbn, CBN_OF(src), CTX_OF(ctx));
    return *this;
}

m::BigNumber &m::BigNumber::modulo(const BigNumber &src, BNContext &ctx)
{
    BN_mod(m_bn, m_cbn, CBN_OF(src), CTX_OF(ctx));
    return *this;
}

m::BigNumber m::BigNumber::multiplied(const BigNumber &src, BNContext &ctx) const
{
    BigNumber ret;
    BN_mul(BN_OF(ret), m_cbn, CBN_OF(src), CTX_OF(ctx));
    return ret;
}

m::BigNumber m::BigNumber::divided(const BigNumber &src, BNContext &ctx) const
{
    BigNumber ret;
    BN_div(BN_OF(ret), nullptr, m_cbn, CBN_OF(src), CTX_OF(ctx));
    return ret;
}

m::BigNumber m::BigNumber::moduloed(const BigNumber &src, BNContext &ctx) const
{
    BigNumber ret;
    BN_mod(BN_OF(ret), m_cbn, CBN_OF(src), CTX_OF(ctx));
    return ret;
}

bool m::BigNumber::isNegative() const
{
    return BN_is_negative(m_cbn) != 0;
}

m::BigNumber &m::BigNumber::setNegative(bool negative)
{
    BN_set_negative(m_bn, negative ? 1 : 0);
    return *this;
}

uint64_t m::BigNumber::word() const
{
    return BN_get_word(m_cbn);
}

m::BigNumber &m::BigNumber::setBytes(const uint8_t *data, uint32_t len)
{
    BN_bin2bn(data, static_cast<int>(len), m_bn);
    return *this;
}

m::BigNumber &m::BigNumber::parse(const String &data, bool isHex)
{
    BIGNUM *bn = nullptr;

    if(isHex)
        BN_hex2bn(&bn, data.raw());
    else
        BN_dec2bn(&bn, data.raw());

    BN_copy(m_bn, bn);
    BN_clear_free(bn);
    return *this;
}

uint32_t m::BigNumber::size() const
{
    return static_cast<uint32_t>(BN_num_bytes(m_cbn));
}

bool m::BigNumber::bytes(uint8_t *dst, uint32_t len) const
{
    if(len < static_cast<uint32_t>(BN_num_bytes(m_cbn)))
        return false;

    BN_bn2bin(m_cbn, dst);
    return true;
}

void m::BigNumber::bytes(uint8_t *dst) const
{
    BN_bn2bin(m_cbn, dst);
}

m::String m::BigNumber::toString(bool hex) const
{
    char *str;
    if(hex)
        str = BN_bn2hex(m_cbn);
    else
        str = BN_bn2dec(m_cbn);

    String ret(str);
    OPENSSL_free(str);

    return ret;
}

m::BigNumber &m::BigNumber::setBit(int bit)
{
    BN_set_bit(m_bn, bit);
    return *this;
}

m::BigNumber &m::BigNumber::setBit(int bit, bool val)
{
    if(val)
        BN_set_bit(m_bn, bit);
    else
        BN_clear_bit(m_bn, bit);

    return *this;
}

m::BigNumber &m::BigNumber::clearBit(int bit)
{
    BN_clear_bit(m_bn, bit);
    return *this;
}

bool m::BigNumber::isBitSet(int bit) const
{
    return BN_is_bit_set(m_cbn, bit) != 0;
}

m::BigNumber &m::BigNumber::operator = (const BigNumber &src)
{
    if(m_bn_ == src.m_bn_)
        return *this;

    BN_copy(m_bn, CBN_OF(src));
    return *this;
}

m::BigNumber &m::BigNumber::operator = (BigNumber &&src)
{
    BN_clear_free(m_bn);
    m_bn = BN_OF(src);
    src.m_bn_ = nullptr;
    return *this;
}

bool m::BigNumber::operator < (const BigNumber &src) const
{
    return BN_cmp(m_cbn, CBN_OF(src)) < 0;
}

bool m::BigNumber::operator <= (const BigNumber &src) const
{
    return BN_cmp(m_cbn, CBN_OF(src)) <= 0;
}

bool m::BigNumber::operator == (const BigNumber &src) const
{
    return BN_cmp(m_cbn, CBN_OF(src)) == 0;
}

bool m::BigNumber::operator == (uint64_t word) const
{
    return BN_is_word(m_cbn, word);
}

bool m::BigNumber::operator >= (const BigNumber &src) const
{
    return BN_cmp(m_cbn, CBN_OF(src)) >= 0;
}

bool m::BigNumber::operator > (const BigNumber &src) const
{
    return BN_cmp(m_cbn, CBN_OF(src)) > 0;
}

int m::BigNumber::compare(const BigNumber &src) const
{
    return BN_cmp(m_cbn, CBN_OF(src));
}

bool m::BigNumber::isZero() const
{
    return BN_is_zero(m_cbn);
}

bool m::BigNumber::isOne() const
{
    return BN_is_one(m_cbn);
}

bool m::BigNumber::isOdd() const
{
    return BN_is_odd(m_cbn);
}

m::BigNumber &m::BigNumber::operator++ ()
{
    BN_add_word(m_bn, 1);
    return *this;
}

m::BigNumber &m::BigNumber::operator-- ()
{
    BN_sub_word(m_bn, 1);
    return *this;
}

m::BigNumber m::BigNumber::operator++ (int)
{
    BigNumber ret(*this);
    BN_add_word(m_bn, 1);
    return ret;
}

m::BigNumber m::BigNumber::operator-- (int)
{
    BigNumber ret(*this);
    BN_sub_word(m_bn, 1);
    return ret;
}

m::BigNumber &m::BigNumber::square()
{
    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    BN_sqr(m_bn, m_cbn, ctx);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    return *this;
}

m::BigNumber m::BigNumber::squared() const
{
    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    BigNumber ret;
    BN_sqr(BN_OF(ret), m_cbn, ctx);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);

    return ret;
}

m::BigNumber &m::BigNumber::square(BNContext &ctx)
{
    BN_sqr(m_bn, m_cbn, CTX_OF(ctx));
    return *this;
}

m::BigNumber m::BigNumber::squared(BNContext &ctx) const
{
    BigNumber ret;
    BN_sqr(BN_OF(ret), m_cbn, CTX_OF(ctx));
    return *this;
}

static BIGNUM *g_bnSqrt(BIGNUM *res_, const BIGNUM *src, BN_CTX *ctx)
{
    //https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Binary_numeral_system_.28base_2.29
    mAssert(!BN_is_negative(src), "cannot sqrt negative number");
    int bitCnt = BN_num_bits(src);
    BIGNUM *bit = BN_CTX_get(ctx);
    BIGNUM *num = BN_CTX_get(ctx);
    BIGNUM *rpb = BN_CTX_get(ctx);
    BIGNUM *res = (res_ == nullptr) ? BN_new() : BN_CTX_get(ctx); //If res_ is null, allocate a new. Use a temporary one otherwise.

    BN_zero(bit);
    BN_set_bit(bit, bitCnt - 2);
    BN_copy(num, src);

    while(BN_cmp(bit, num) > 0)
        BN_rshift(bit, bit, 2);

    while(!BN_is_zero(bit)) {
        BN_add(rpb, res, bit);
        BN_rshift(res, res, 1);

        if(BN_cmp(num, rpb) >= 0) {
            BN_sub(num, num, rpb);
            BN_add(res, res, bit);
        }

        BN_rshift(bit, bit, 2);
    }

    if(res_ != nullptr)
        BN_copy(res_, res); //If we used a temporary one, copy it in res_

    return res_ == nullptr ? res : res_; //If we created a new one, return it.
}

m::BigNumber &m::BigNumber::sqrt()
{
    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    g_bnSqrt(m_bn, m_cbn, ctx);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);

    return *this;
}

m::BigNumber &m::BigNumber::sqrt(BNContext &ctx)
{
    g_bnSqrt(m_bn, m_cbn, CTX_OF(ctx));
    return *this;
}

m::BigNumber m::BigNumber::sqrted() const
{
    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    BIGNUM *ret = g_bnSqrt(nullptr, m_cbn, ctx);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);

    return BigNumber(ret);
}

m::BigNumber m::BigNumber::sqrted(BNContext &ctx) const
{
    return BigNumber(g_bnSqrt(nullptr, m_cbn, CTX_OF(ctx)));
}

m::BigNumber m::BigNumber::gcd(const BigNumber &src, BNContext &ctx) const
{
    BigNumber ret;
    BN_gcd(BN_OF(ret), m_cbn, CBN_OF(src), CTX_OF(ctx));

    return ret;
}

m::BigNumber m::BigNumber::modInverse(const BigNumber &src, BNContext &ctx) const
{
    BigNumber ret;
    BN_mod_inverse(BN_OF(ret), m_cbn, CBN_OF(src), CTX_OF(ctx));

    return ret;
}

void *m::BigNumber::rawCopy() const
{
    return BN_dup(m_cbn);
}

#endif
