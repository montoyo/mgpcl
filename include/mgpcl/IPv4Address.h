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
#include "mgpcl/INet.h"
#include "mgpcl/Assert.h"
#include "mgpcl/String.h"
#include <cstdint>

namespace m
{
	enum DNSResolveError
	{
		kRE_NoError = 0,
		kRE_NonIPv4Host,
		kRE_TryLater,
		kRE_DNSFailure,
		kRE_OutOfMemory,
		kRE_UnknownHost,
		kRE_SystemError,
		kRE_UnknownError
	};

	enum AddressFormatError
	{
		kAFE_NoError = 0,
		kAFE_InvalidAddressNumber,
		kAFE_InvalidPortNumber,
		kAFE_InvalidFormat,
		kAFE_MissingPort
	};

	class IPv4Address
	{
	public:
		IPv4Address();
		IPv4Address(uint16_t port); //Creates an "any" address
		IPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port);

		IPv4Address(const struct sockaddr_in &sain)
		{
			mDebugAssert(sain.sin_family == AF_INET, "invalid sockaddr_in");
			m_sain = sain;
		}

		DNSResolveError resolve(const String &str, uint16_t port = 0);
		AddressFormatError parse(const String &str); //Parse from format #.#.#.#:# but also *:# (which is any address)
		AddressFormatError parse(const String &str, uint16_t defPort);

		static IPv4Address localhost(uint16_t port)
		{
			return IPv4Address(127, 0, 0, 1, port);
		}

		void setPort(uint16_t p)
		{
			m_sain.sin_port = ((p & 0xFF00) >> 8) | ((p & 0x00FF) << 8);
		}

		uint16_t port() const
		{
			return ((m_sain.sin_port & 0xFF00) >> 8) | ((m_sain.sin_port & 0x00FF) << 8);
		}

		void setAddr(int idx, uint8_t val)
		{
			mDebugAssert(idx >= 0 && idx < 4, "address index out of [0;4[ range");
			reinterpret_cast<uint8_t*>(&m_sain.sin_addr.s_addr)[idx] = val;
		}

		void setAddr(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
		{
			//Don't forget this is little endian
			m_sain.sin_addr.s_addr = static_cast<uint32_t>(a) | (static_cast<uint32_t>(b) << 8) | (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24);
		}

		uint8_t addr(int idx) const
		{
			mDebugAssert(idx >= 0 && idx < 4, "address index out of [0;4[ range");
			return reinterpret_cast<const uint8_t*>(&m_sain.sin_addr.s_addr)[idx];
		}

		bool operator == (const IPv4Address &src) const
		{
			return m_sain.sin_addr.s_addr == src.m_sain.sin_addr.s_addr && m_sain.sin_port == src.m_sain.sin_port;
		}

		bool operator != (const IPv4Address &src) const
		{
			return m_sain.sin_addr.s_addr != src.m_sain.sin_addr.s_addr || m_sain.sin_port != src.m_sain.sin_port;
		}

		bool isAny() const
		{
			return m_sain.sin_addr.s_addr == INADDR_ANY;
		}

		String toString(bool withPort = true) const;

		const struct sockaddr_in *raw() const
		{
			return &m_sain;
		}

		struct sockaddr_in *raw()
		{
			return &m_sain;
		}

		static socklen_t rawSize()
		{
			return sizeof(struct sockaddr_in);
		}

		IPv4Address &operator = (const struct sockaddr_in &sain)
		{
			mDebugAssert(sain.sin_family == AF_INET, "invalid sockaddr_in");
			m_sain = sain;
			return *this;
		}

	private:
		struct sockaddr_in m_sain;
	};
}
