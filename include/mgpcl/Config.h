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

/* MGPCL VERSION */
#define MGPCL_VERSION_MAJOR 1
#define MGPCL_VERSION_MINOR 0

/* MGPCL MACROS & CONSTANTS */
#define _MGPCL_STR(a) #a
#define _MGPCL_VERSTR(a, b) _MGPCL_STR(a) "." _MGPCL_STR(b)
#define MGPCL_VERSION_STRING _MGPCL_VERSTR(MGPCL_VERSION_MAJOR, MGPCL_VERSION_MINOR)

#if defined(_WIN32)
#define MGPCL_WIN
#elif defined(__GNUC__)
#define MGPCL_LINUX
#else
#error "Unsupported platform"
#endif

//This was supposed to be used for DLL/SO, but it's better if it stays static
#define MGPCL_PREFIX

//Other settings
//#define MGPCL_NO_GUI
//#define MGPCL_NO_SSL

