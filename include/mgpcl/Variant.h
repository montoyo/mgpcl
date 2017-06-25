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
#include <cstdint>

namespace m
{

	enum class VariantType
	{
		Null = 0,
		Int,
		Short,
		Char,
		UInt,
		UShort,
		Byte,
		Double,
		Float,
		String
	};

	class MGPCL_PREFIX Variant
	{
	public:
		Variant();
		Variant(int i);
		Variant(short i);
		Variant(char i);
		Variant(uint32_t i);
		Variant(uint16_t i);
		Variant(uint8_t i);
		Variant(double i);
		Variant(float i);
		Variant(const String &i);
		~Variant();

		int asInt() const;
		short asShort() const;
		char asChar() const;
		uint32_t asUInt() const;
		uint16_t asUShort() const;
		uint8_t asByte() const;
		double asDouble() const;
		float asFloat() const;
		const String &asString() const;

		Variant &operator = (int i);
		Variant &operator = (short i);
		Variant &operator = (char i);
		Variant &operator = (uint32_t i);
		Variant &operator = (uint16_t i);
		Variant &operator = (uint8_t i);
		Variant &operator = (double i);
		Variant &operator = (float i);
		Variant &operator = (const String &i);

		Variant &setNull();

		VariantType getType() const
		{
			return m_type;
		}

		bool isNull() const
		{
			return m_type == VariantType::Null;
		}

	private:
		void destroy();

		template<typename T> const T &getRef(VariantType type) const
		{
			if(type != m_type)
				abort();

			return *reinterpret_cast<T*>(m_value);
		}

		template<typename T> void allocate(VariantType type, const T &val)
		{
			m_type = type;
			m_value = new uint8_t[sizeof(T)];
			new(m_value) T(val);
		}

		template<typename T> Variant &set(VariantType type, const T &val)
		{
			if(type != m_type) {
				destroy();
				allocate<T>(type, val);
			} else
				*reinterpret_cast<T*>(m_value) = val;

			return *this;
		}

		VariantType m_type;
		uint8_t *m_value;
	};

}
