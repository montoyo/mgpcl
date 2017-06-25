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

#include "mgpcl/Assert.h"
#include "mgpcl/String.h"
#include "mgpcl/Math.h"

#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <iostream>
#include <cstdlib>
#endif

void m::debugBreak(const char *fname, int line, const char *str)
{
	String txt(fname);
	int pos = Math::maximum(txt.lastIndexOf('/'), txt.lastIndexOf('\\'));

	if(pos > 0)
		txt = txt.substr(pos + 1);

	txt += '@';
	txt += String::fromInteger(line);
	txt += ": ";
	txt += str;

#ifdef MGPCL_WIN
	txt += "\r\n";

	OutputDebugStringA(txt.raw());
	DebugBreak();
#else
	std::cerr << txt.raw() << std::endl;
	std::abort();
#endif
}
