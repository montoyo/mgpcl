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

#include "mgpcl/Variant.h"

m::Variant::Variant()
{
	m_type = VariantType::Null;
	m_value = nullptr;
}

m::Variant::Variant(int i)
{
	allocate<int>(VariantType::Int, i);
}

m::Variant::Variant(short i)
{
	allocate<short>(VariantType::Short, i);
}

m::Variant::Variant(char i)
{
	allocate<char>(VariantType::Char, i);
}

m::Variant::Variant(uint32_t i)
{
	allocate<uint32_t>(VariantType::UInt, i);
}

m::Variant::Variant(uint16_t i)
{
	allocate<uint16_t>(VariantType::UShort, i);
}

m::Variant::Variant(uint8_t i)
{
	allocate<uint8_t>(VariantType::Byte, i);
}

m::Variant::Variant(double i)
{
	allocate<double>(VariantType::Double, i);
}

m::Variant::Variant(float i)
{
	allocate<float>(VariantType::Float, i);
}

m::Variant::Variant(const String &i)
{
	allocate<String>(VariantType::String, i);
}

m::Variant::~Variant()
{
	destroy();
}

void m::Variant::destroy()
{
	switch(m_type) {
	case VariantType::Null:
		break;

	case VariantType::String:
		reinterpret_cast<String*>(m_value)->~TString();

	default:
		delete[] m_value;
	}
}

int m::Variant::asInt() const
{
	return getRef<int>(VariantType::Int);
}

short m::Variant::asShort() const
{
	return getRef<short>(VariantType::Short);
}

char m::Variant::asChar() const
{
	return getRef<char>(VariantType::Char);
}

uint32_t m::Variant::asUInt() const
{
	return getRef<uint32_t>(VariantType::UInt);
}

uint16_t m::Variant::asUShort() const
{
	return getRef<uint16_t>(VariantType::UShort);
}

uint8_t m::Variant::asByte() const
{
	return getRef<uint8_t>(VariantType::Byte);
}

double m::Variant::asDouble() const
{
	return getRef<double>(VariantType::Double);
}

float m::Variant::asFloat() const
{
	return getRef<float>(VariantType::Float);
}

const m::String &m::Variant::asString() const
{
	return getRef<String>(VariantType::String);
}

m::Variant &m::Variant::operator = (int i)
{
	return set<int>(VariantType::Int, i);
}

m::Variant &m::Variant::operator = (short i)
{
	return set<short>(VariantType::Short, i);
}

m::Variant &m::Variant::operator = (char i)
{
	return set<char>(VariantType::Char, i);
}

m::Variant &m::Variant::operator = (uint32_t i)
{
	return set<uint32_t>(VariantType::UInt, i);
}

m::Variant &m::Variant::operator = (uint16_t i)
{
	return set<uint16_t>(VariantType::UShort, i);
}

m::Variant &m::Variant::operator = (uint8_t i)
{
	return set<uint8_t>(VariantType::Byte, i);
}

m::Variant &m::Variant::operator = (double i)
{
	return set<double>(VariantType::Double, i);
}

m::Variant &m::Variant::operator = (float i)
{
	return set<float>(VariantType::Float, i);
}

m::Variant &m::Variant::operator = (const String &i)
{
	return set<String>(VariantType::String, i);
}

m::Variant &m::Variant::setNull()
{
	if(m_type != VariantType::Null) {
		destroy();

		m_type = VariantType::Null;
		m_value = nullptr;
	}

	return *this;
}
