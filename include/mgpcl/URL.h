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
#include <cstdint>

namespace m
{
	enum URLParseError
	{
		kUPE_NoError = 0,
		kUPE_MissingProtocol,
		kUPE_MissingHost,
		kUPE_InvalidPort,
		kUPE_InvalidCharacter,
		kUPE_SourceInvalid, //Only valid for URL::parseRelative()
		kUPE_InvalidFormat
	};

	//NOTE: This class was designed to handle HTTP(S) URLs
	//		and thus does not entierely follow RFC 3986
	//		For instance, it won't parse URLs like proto:data
	//		because it's missing slashes after the protocol
	//		separator
	class URL
	{
	public:
		URL()
		{
			m_port = 0;
		}

		URL(const String &url)
		{
			m_port = 0;
			parse(url);
		}

		URL(const String &proto, const String &host, uint16_t port, const String &loc)
		{
			m_proto = proto;
			m_host = host;
			m_port = port;
			m_location = loc;
		}

		URLParseError parse(const String &url);
		URLParseError parseRelative(const URL &src, const String &url);
		String toString() const;
		static String encode(const String &u);
		static bool decode(const String &u, String &dst);

		void setProtocol(const String &proto)
		{
			m_proto = proto;
		}

		void setHost(const String &host)
		{
			m_host = host;
		}

		void setPort(const String &port)
		{
			m_proto = port;
		}

		void setLocation(const String &loc)
		{
			m_location = loc;
		}

		const String &protocol() const
		{
			return m_proto;
		}

		const String &host() const
		{
			return m_host;
		}

		uint16_t port() const
		{
			return m_port;
		}

		const String &location() const
		{
			return m_location;
		}

		bool operator == (const URL &src) const
		{
			//NOTE: Case sensitive host and percent encoding
			return m_proto == src.m_proto && m_host == src.m_host && m_port == src.m_port && m_location == src.m_location;
		}

		bool operator != (const URL &src) const
		{
			//NOTE: Case sensitive host and percent encoding
			return m_proto != src.m_proto || m_host != src.m_host || m_port != src.m_port || m_location != src.m_location;
		}

		bool isValid() const
		{
			return !m_proto.isEmpty() && !m_host.isEmpty() && m_port != 0;
		}

		//TODO: Encode/decode arguments

	private:
		static bool hexDecode(char &c);
		static bool checkStringRange(const String &str, int a, int b, bool dotOk);

		String m_proto;
		String m_host;
		uint16_t m_port;
		String m_location;
	};
}
