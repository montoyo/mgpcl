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

namespace m
{
	template<class T> class DefaultHasher
	{
	public:
		static int hash(const T &src)
		{
			return src.hash();
		}
	};

	template<typename T> class StaticCastHasher
	{
	public:
		static int hash(T i)
		{
			return static_cast<int>(i);
		}
	};

	template<> class DefaultHasher<int> : public StaticCastHasher<int>
	{
	};

	template<> class DefaultHasher<short> : public StaticCastHasher<short>
	{
	};

	template<> class DefaultHasher<char> : public StaticCastHasher<char>
	{
	};

	template<> class DefaultHasher<long> : public StaticCastHasher<long>
	{
	};

	template<> class DefaultHasher<unsigned int> : public StaticCastHasher<unsigned int>
	{
	};

	template<> class DefaultHasher<unsigned short> : public StaticCastHasher<unsigned short>
	{
	};

	template<> class DefaultHasher<unsigned char> : public StaticCastHasher<unsigned char>
	{
	};

	template<> class DefaultHasher<unsigned long> : public StaticCastHasher<unsigned long>
	{
	};

	template<> class DefaultHasher<float>
	{
	public:
		static int hash(float i)
		{
			static_assert(sizeof(float) == sizeof(int), "sizeof(float) != sizeof(int)");
			return *reinterpret_cast<int*>(&i);
		}
	};

}
