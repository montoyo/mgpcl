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
#include <cstring>

namespace m
{
	namespace Mem
	{
		inline void *copy(void *dst, const void *src, size_t sz)
		{
			return memcpy(dst, src, sz);
		}

		inline void *move(void *dst, const void *src, size_t sz)
		{
			return memmove(dst, src, sz);
		}

		inline int cmp(const void *a, const void *b, size_t sz)
		{
			return memcmp(a, b, sz);
		}

		inline void *zero(void *dst, size_t sz)
		{
			return memset(dst, 0, sz);
		}

		template<typename T> T &zero(T &obj)
		{
			return *static_cast<T*>(memset(&obj, 0, sizeof(T)));
		}

	}
}
