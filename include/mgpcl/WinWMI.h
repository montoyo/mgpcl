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
#include "Config.h"
#include "String.h"
#include "Atomic.h"

#ifdef M_WMI_DECLARE
#define M_WMI_PREFIX
#else
#define M_WMI_PREFIX extern
#endif

#ifdef MGPCL_WIN
#include <comdef.h>
#include <Wbemidl.h>

namespace m
{
	class WMIResult;

	namespace wmi
	{
		M_WMI_PREFIX bool acquire();
		M_WMI_PREFIX WMIResult *query(const char *q);
		M_WMI_PREFIX const String &lastError();
		M_WMI_PREFIX void release();
	}

	//Note: I think we can remove the ref-counting bs here...
	class WMIResult
	{
		friend WMIResult *wmi::query(const char *q);

	public:
		bool next();
		void addRef();
		void releaseRef();
		String getString(LPCWSTR key);
		uint32_t getUInt32(LPCWSTR key);

	private:
		WMIResult(IEnumWbemClassObject *ienum);
		WMIResult()
		{
		}

		~WMIResult()
		{
		}

		Atomic m_refs;
		IEnumWbemClassObject *m_enumerator;
		IWbemClassObject *m_entry;
	};

}

#endif
