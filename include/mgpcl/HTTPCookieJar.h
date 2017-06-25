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
#include "URL.h"
#include "HashMap.h"
#include "Date.h"
#include "DataSerializer.h"
#include <limits>

#ifdef max
#undef max
#endif

namespace m
{
	
	class HTTPCookie
	{
	public:
		HTTPCookie()
		{
			m_expiry = std::numeric_limits<time_t>::max();
			m_maxAge = ~0;
			m_secure = false;
			m_httpOnly = false;
			m_sessionLocal = true;
			m_creation = Date::unixTime();
		}

		bool parse(const String &hdr); //Parses from header string
		bool isValid(time_t now) const;
		bool isSuitableFor(const URL &u) const;
		void serialize(DataSerializer &s) const;
		void deserialize(DataDeserializer &s);

		const String &name() const
		{
			return m_name;
		}

		void setName(const String &name)
		{
			m_name = name;
		}

		const String &value() const
		{
			return m_value;
		}

		void setValue(const String &value)
		{
			m_value = value;
		}

		const String &domain() const
		{
			return m_domain;
		}

		void setDomain(const String &domain)
		{
			m_domain = domain;
		}

		const String &path() const
		{
			return m_path;
		}

		void setPath(const String &path)
		{
			m_path = path;
		}

		time_t expiry() const
		{
			return m_expiry;
		}

		void setExpiry(time_t expiry)
		{
			m_expiry = expiry;
		}

		uint32_t maxAge() const
		{
			return m_maxAge;
		}

		void setMaxAge(uint32_t maxAge)
		{
			m_maxAge = maxAge;
		}

		bool isSecure() const
		{
			return m_secure;
		}

		void setSecure(bool secure)
		{
			m_secure = secure;
		}

		bool isHttpOnly() const
		{
			return m_httpOnly;
		}

		void setHttpOnly(bool httpOnly)
		{
			m_httpOnly = httpOnly;
		}

		time_t creationTime() const
		{
			return m_creation;
		}

		void setCreationTime(time_t ct)
		{
			m_creation = ct;
		}

		bool isSessionLocal() const
		{
			return m_sessionLocal;
		}

		void setSessionLocal(bool sl)
		{
			m_sessionLocal = sl;
		}

		bool isValid() const
		{
			return !m_name.isEmpty();
		}

	private:
		time_t m_creation;
		String m_name;
		String m_value;

		time_t m_expiry;
		uint32_t m_maxAge;
		String m_domain;
		String m_path;
		bool m_secure;
		bool m_httpOnly;
		bool m_sessionLocal;
	};

	class HTTPCookieJar
	{
	public:
		typedef HashMap<String, HTTPCookie>::Iterator Iterator;
		typedef HashMap<String, HTTPCookie>::Pair Pair;

		HTTPCookie cookie(const String &name)
		{
			return m_content[name];
		}

		const String &cookieValue(const String &name)
		{
			return m_content[name].value();
		}

		String cookieValue(const String &name) const
		{
			if(m_content.hasKey(name))
				return m_content.get(name).value();
			else
				return String();
		}

		bool hasCookie(const String &name) const
		{
			return m_content.hasKey(name);
		}

		void putCookie(const HTTPCookie &c)
		{
			m_content[c.name()] = c;
		}

		Iterator begin()
		{
			return m_content.begin();
		}

		Iterator end()
		{
			return m_content.end();
		}

		void clear()
		{
			m_content.clear();
		}

		bool isEmpty() const
		{
			return m_content.isEmpty();
		}

		bool parseAndPutCookie(const String &data);
		void serialize(DataSerializer &s);
		void deserialize(DataDeserializer &s);

	private:
		HashMap<String, HTTPCookie> m_content;
	};

}

