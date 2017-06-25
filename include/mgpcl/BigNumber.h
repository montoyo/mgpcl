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
	class BigNumber;
	class BNContext
	{
		//BNContext is NOT thread safe.
		friend class BigNumber;

	public:
		BNContext();
		BNContext(const BNContext &src);
		BNContext(BNContext &&src);
		~BNContext();

		BNContext &operator = (const BNContext &src);
		BNContext &operator = (BNContext &&src);

		void start();
		void end();

	private:
		void *m_ctx_;
		int *m_refs;
	};

	class BigNumber
	{
	public:
		BigNumber();
		BigNumber(uint64_t word);
		BigNumber(const void *raw);
		BigNumber(const uint8_t *data, uint32_t len);
		BigNumber(const String &data, bool isHex = false);
		BigNumber(const BigNumber &src);
		BigNumber(BigNumber &&src);
		~BigNumber();

		BigNumber &zero();
		BigNumber &setWord(uint64_t word);
		uint64_t word() const;

		BigNumber operator + (const BigNumber &src) const;
		BigNumber operator - (const BigNumber &src) const;
		BigNumber operator * (const BigNumber &src) const; //Slow, better use .multiplied()
		BigNumber operator / (const BigNumber &src) const; //Slow, better use .divided()
		BigNumber operator % (const BigNumber &src) const; //Slow, better use .moduloed()
		BigNumber operator << (int t) const;
		BigNumber operator >> (int t) const;
		BigNumber operator + (uint64_t word) const;
		BigNumber operator - (uint64_t word) const;
		BigNumber operator * (uint64_t word) const;
		BigNumber operator / (uint64_t word) const;
		BigNumber operator % (uint64_t word) const;

		BigNumber &operator += (const BigNumber &src);
		BigNumber &operator -= (const BigNumber &src);
		BigNumber &operator *= (const BigNumber &src); //Slow, better use .multiply()
		BigNumber &operator /= (const BigNumber &src); //Slow, better use .divide()
		BigNumber &operator %= (const BigNumber &src); //Slow, better use .modulo()
		BigNumber &operator <<= (int t);
		BigNumber &operator >>= (int t);
		BigNumber &operator += (uint64_t word);
		BigNumber &operator -= (uint64_t word);
		BigNumber &operator *= (uint64_t word);
		BigNumber &operator /= (uint64_t word);
		BigNumber &operator %= (uint64_t word);

		BigNumber &multiply(const BigNumber &src, BNContext &ctx);
		BigNumber &divide(const BigNumber &src, BNContext &ctx);
		BigNumber &modulo(const BigNumber &src, BNContext &ctx);
		BigNumber multiplied(const BigNumber &src, BNContext &ctx) const;
		BigNumber divided(const BigNumber &src, BNContext &ctx) const;
		BigNumber moduloed(const BigNumber &src, BNContext &ctx) const; //yes yes english

		bool isNegative() const;
		BigNumber &setNegative(bool negative = true);
		BigNumber &setBytes(const uint8_t *data, uint32_t len); //Does not handle the sign, also: big endian
		BigNumber &parse(const String &data, bool isHex = false);

		uint32_t size() const;
		bool bytes(uint8_t *dst, uint32_t len) const; //Stores the absolute value (big endian)
		void bytes(uint8_t *dst) const; //Also stores the absolute value (big endian)
		String toString(bool hex = false) const;

		BigNumber &setBit(int bit);
		BigNumber &setBit(int bit, bool val);
		BigNumber &clearBit(int bit);
		bool isBitSet(int bit) const;

		BigNumber &operator = (const BigNumber &src);
		BigNumber &operator = (BigNumber &&src);

		BigNumber &operator++ ();
		BigNumber &operator-- ();
		BigNumber operator++ (int);
		BigNumber operator-- (int);

		bool operator < (const BigNumber &src) const;
		bool operator <= (const BigNumber &src) const;
		bool operator == (const BigNumber &src) const;
		bool operator == (uint64_t word) const;
		bool operator >= (const BigNumber &src) const;
		bool operator > (const BigNumber &src) const;

		int compare(const BigNumber &src) const;
		bool isZero() const;
		bool isOne() const;
		bool isOdd() const;

		BigNumber &square();
		BigNumber &square(BNContext &ctx);
		BigNumber squared() const;
		BigNumber squared(BNContext &ctx) const;

		BigNumber &sqrt();
		BigNumber &sqrt(BNContext &ctx);
		BigNumber sqrted() const;
		BigNumber sqrted(BNContext &ctx) const;

		BigNumber gcd(const BigNumber &src, BNContext &ctx) const;
		BigNumber modInverse(const BigNumber &src, BNContext &ctx) const;

		void *rawCopy() const;
		void *raw() const
		{
			return m_bn_;
		}

	private:
		void *m_bn_;
	};
}

#endif
