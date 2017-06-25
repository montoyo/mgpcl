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

#include "mgpcl/Time.h"

#ifdef MGPCL_WIN

static double g_freq;
static LARGE_INTEGER g_offset;

void m::time::initTime()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	freq.QuadPart /= 1000;
	g_freq = static_cast<double>(freq.QuadPart);

	QueryPerformanceCounter(&g_offset);
}

double m::time::getTimeMs()
{
	LARGE_INTEGER ctime;
	QueryPerformanceCounter(&ctime);

	ctime.QuadPart -= g_offset.QuadPart;
	return static_cast<double>(ctime.QuadPart) / g_freq;
}

#else
#include <ctime>

static time_t g_offset;

void m::time::initTime()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    g_offset = ts.tv_sec;
}

double m::time::getTimeMs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    double ret = static_cast<double>(ts.tv_sec - g_offset) * 1000.0;
    ret += static_cast<double>(ts.tv_nsec / 1000) / 1000.0;

    return ret;
}

uint32_t m::time::getTimeMsUInt()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	ts.tv_sec *= 1000;
	ts.tv_nsec /= 1000000;

	return static_cast<uint32_t>(ts.tv_sec + ts.tv_nsec);
}

#endif
