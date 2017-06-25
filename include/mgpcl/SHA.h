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
	enum SHAVersion
	{
		kSHAV_None = 0,
		//kSHAV_Sha0, //Supported removed
		kSHAV_Sha1,
		kSHAV_Sha224,
		kSHAV_Sha256,
		kSHAV_Sha384,
		kSHAV_Sha512
	};

	class SHA
	{
	public:
		SHA();
		SHA(SHAVersion ver);
		SHA(const SHA &src);
		SHA(SHA &&src);
		~SHA();

		bool init(SHAVersion ver);
		void reset();
		void update(const uint8_t *src, uint32_t sz);
		void update(const char *str, int len = -1);
		void update(const String &str);
		bool digest(uint8_t *dst, uint32_t sz);
		void digest(String &dst); //Appends the hex string corresponding to the digest to 'dst'.
		uint8_t *digest(); //Free it using delete[]

		uint32_t digestSize() const;
		static uint32_t digestSize(SHAVersion ver);

		SHAVersion version() const
		{
			return m_ver;
		}

		bool isValid() const
		{
			return m_ver != kSHAV_None && m_ctx_ != nullptr;
		}

		SHA &operator = (const SHA &src);
		SHA &operator = (SHA &&src);

		static bool quick(SHAVersion ver, const uint8_t *data, uint32_t len, uint8_t *result, uint32_t resultSz);
		static String quick(SHAVersion ver, const uint8_t *data, uint32_t len);

		static bool quick(SHAVersion ver, const String &data, uint8_t *result, uint32_t resultSz)
		{
			return quick(ver, reinterpret_cast<const uint8_t*>(data.raw()), static_cast<uint32_t>(data.length()), result, resultSz);
		}

		static String quick(SHAVersion ver, const String &data)
		{
			return quick(ver, reinterpret_cast<const uint8_t*>(data.raw()), static_cast<uint32_t>(data.length()));
		}

	private:
		SHAVersion m_ver;
		void *m_ctx_;
	};
}

#endif
