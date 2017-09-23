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

namespace m
{
	class CPUInfo
	{
	public:
		bool isValid() const
		{
			return m_valid;
		}

		bool operator ! () const
		{
			return !m_valid;
		}

		//If !isValid(), use this to get the error string
		String error() const
		{
			return m_error;
		}

		const String &name() const
		{
			return m_name;
		}

		const String &vendor() const
		{
			return m_vendor;
		}

		uint32_t numCores() const
		{
			return m_cores;
		}

		//In MHz
		uint32_t maxFrequency() const
		{
			return m_maxFreq;
		}

		static CPUInfo fetch();

	private:
		CPUInfo()
		{
			m_valid = true;
		}

		CPUInfo(const char *err) : m_error(err)
		{
			m_valid = false;
		}

		CPUInfo(const String &err) : m_error(err)
		{
			m_valid = false;
		}

		bool m_valid;
		String m_error;
		String m_name;
		String m_vendor;
		uint32_t m_cores;
		uint32_t m_maxFreq;
	};
}
