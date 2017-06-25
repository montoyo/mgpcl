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

#include "mgpcl/JSON.h"
#include "mgpcl/List.h"
#include "mgpcl/BasicParser.h"

m::JSONElement::JSONElement(const JSONElement &src) : m_name(src.m_name)
{
	m_type = src.m_type;

	switch(m_type) {
	case kJT_Boolean:
		dataAs<bool>() = src.dataAs<bool>();
		break;

	case kJT_Number:
		dataAs<double>() = src.dataAs<double>();
		break;

	case kJT_String:
		new(&dataAs<String>()) String(src.dataAs<String>());
		break;

	case kJT_Array:
		new(&dataAs<JSONList>()) JSONList(src.dataAs<JSONList>());
		break;

	case kJT_Object:
		new(&dataAs<JSONMap>()) JSONMap(src.dataAs<JSONMap>());
		break;

	default:
		break;
	}
}

m::JSONElement::JSONElement(JSONElement &&src) : m_name(std::move(src.m_name))
{
	m_type = src.m_type;

	switch(m_type) {
	case kJT_Boolean:
		dataAs<bool>() = src.dataAs<bool>();
		break;

	case kJT_Number:
		dataAs<double>() = src.dataAs<double>();
		break;

	case kJT_String:
		new(&dataAs<String>()) String(std::move(src.dataAs<String>()));
		break;

	case kJT_Array:
		new(&dataAs<JSONList>()) JSONList(std::move(src.dataAs<JSONList>()));
		break;

	case kJT_Object:
		new(&dataAs<JSONMap>()) JSONMap(std::move(src.dataAs<JSONMap>()));
		break;

	default:
		break;
	}
}

void m::JSONElement::allocate()
{
	switch(m_type) {
	case kJT_String:
		new(&dataAs<String>()) String;
		break;
		
	case kJT_Array:
		new(&dataAs<JSONList>()) JSONList;
		break;

	case kJT_Object:
		new(&dataAs<JSONMap>()) JSONMap;
		break;

	default:
		break;
	}
}

void m::JSONElement::deallocate()
{
	if(m_type == kJT_String)
		dataAs<String>().~String();
	else if(m_type == kJT_Array)
		dataAs<JSONList>().~JSONList();
	else if(m_type == kJT_Object)
		dataAs<JSONMap>().~JSONMap();

	m_type = kJT_Null;
}

m::JSONElement &m::JSONElement::operator = (const JSONElement &src)
{
	if(&src == this)
		return *this;

	deallocate();
	m_type = src.m_type;
	m_name = src.m_name;

	switch(m_type) {
	case kJT_Boolean:
		dataAs<bool>() = src.dataAs<bool>();
		break;

	case kJT_Number:
		dataAs<double>() = src.dataAs<double>();
		break;

	case kJT_String:
		new(&dataAs<String>()) String(src.dataAs<String>());
		break;

	case kJT_Array:
		new(&dataAs<JSONList>()) JSONList(src.dataAs<JSONList>());
		break;

	case kJT_Object:
		new(&dataAs<JSONMap>()) JSONMap(src.dataAs<JSONMap>());
		break;

	default:
		break;
	}

	return *this;
}

m::JSONElement &m::JSONElement::operator = (JSONElement &&src)
{
	deallocate();
	m_type = src.m_type;
	m_name = std::move(src.m_name);

	switch(m_type) {
	case kJT_Boolean:
		dataAs<bool>() = src.dataAs<bool>();
		break;

	case kJT_Number:
		dataAs<double>() = src.dataAs<double>();
		break;

	case kJT_String:
		new(&dataAs<String>()) String(std::move(src.dataAs<String>()));
		break;

	case kJT_Array:
		new(&dataAs<JSONList>()) JSONList(std::move(src.dataAs<JSONList>()));
		break;

	case kJT_Object:
		new(&dataAs<JSONMap>()) JSONMap(std::move(src.dataAs<JSONMap>()));
		break;

	default:
		break;
	}

	return *this;
}

m::JSONElement &m::JSONElement::operator = (bool data)
{
	if(m_type != kJT_Boolean) {
		deallocate();
		m_type = kJT_Boolean;
	}

	dataAs<bool>() = data;
	return *this;
}

m::JSONElement &m::JSONElement::operator = (int data)
{
	if(m_type != kJT_Number) {
		deallocate();
		m_type = kJT_Number;
	}

	dataAs<double>() = static_cast<double>(data);
	return *this;
}

m::JSONElement &m::JSONElement::operator = (double data)
{
	if(m_type != kJT_Number) {
		deallocate();
		m_type = kJT_Number;
	}

	dataAs<double>() = data;
	return *this;
}

m::JSONElement &m::JSONElement::operator = (const String &data)
{
	if(m_type == kJT_String)
		dataAs<String>() = data;
	else {
		deallocate();
		m_type = kJT_String;
		new(&dataAs<String>()) String(data);
	}

	return *this;
}

m::JSONElement &m::JSONElement::operator = (const char *data)
{
	if(m_type == kJT_String)
		dataAs<String>() = data;
	else {
		deallocate();
		m_type = kJT_String;
		new(&dataAs<String>()) String(data);
	}

	return *this;
}

static bool g_m_json_parseString(m::BasicParser &src, m::String &val, m::String &err, char c)
{
	char begin = c;
	bool escaped = false;

	while(true) {
		int ic = src.nextCharRaw();
		if(ic < 0) {
			err = "couldn't read from input";
			return false;
		}

		c = static_cast<char>(ic);
		if(c == '\n' || c == '\r') {
			err = "new line reached before end of string";
			return false;
		} if(escaped) {
			if(c == '\\')
				val += '\\';
			else if(c == 't')
				val += '\t';
			else if(c == 'r')
				val += '\r';
			else if(c == 'n')
				val += '\n';
			else if(c == 'b')
				val += '\b';
			else if(c == 'f')
				val += '\f';
			else if(c == '\'') //Not in the JSON specs
				val += '\'';
			else if(c == '\"')
				val += '\"';
			else {
				//Don't know :O
				val += '\\';
				val += c;
			}

			escaped = false;
		} else if(c == '\\')
			escaped = true;
		else if(c == begin)
			break;
		else
			val += c;
	}

	return true;
}

#define G_M_JSON_ISKEYCHAR(chr) ((chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z') || chr == '_')

static bool g_m_json_parse(m::BasicParser &src, m::JSONElement &dst, m::String &err)
{
	char c;
	{
		int ic = src.nextNonBlankChar();
		if(ic < 0) {
			err = "couldn't read from input";
			return false;
		}

		c = static_cast<char>(ic);
	}

	if(c == '{') {
		//Object
		bool first = true;
		dst = m::JSONElement(m::kJT_Object);

		while(true) {
			int ic = src.nextNonBlankChar();
			if(ic < 0) {
				err = "couldn't read from input";
				return false;
			}

			if(ic == '}')
				break;

			if(first)
				first = false;
			else {
				if(ic != ',') {
					err = "expected comma before next object element";
					return false;
				}

				ic = src.nextNonBlankChar();
				if(ic < 0) {
					err = "couldn't read from input";
					return false;
				}

				if(ic == '}')
					break;
			}

			m::String key(2);
			if(ic == '\'' || ic == '\"') {
				if(!g_m_json_parseString(src, key, err, static_cast<char>(ic)))
					return false;
			} else if(G_M_JSON_ISKEYCHAR(ic)) {
				do {
					key += static_cast<char>(ic);
					ic = src.nextCharRaw();
					if(ic < 0) {
						err = "couldn't read from input";
						return false;
					}
				} while(G_M_JSON_ISKEYCHAR(ic));

				src.undo();
			} else {
				if(ic == ':')
					err = "expected key before value";
				else
					err = "unexpected character before key";

				return false;
			}

			ic = src.nextNonBlankChar();
			if(ic < 0) {
				err = "couldn't read from input";
				return false;
			}

			if(ic != ':') {
				err = "missing ':' before value";
				return false;
			}

			m::JSONElement elem;
			if(!g_m_json_parse(src, elem, err))
				return false;

			elem.setName(key);
			dst.addElement(elem);
		}

		return true;
	} else if(c == '[') {
		//Array
		bool first = true;
		dst = m::JSONElement(m::kJT_Array);

		while(true) {
			int ic = src.nextNonBlankChar();
			if(ic < 0) {
				err = "couldn't read from input";
				return false;
			}

			if(ic == ']')
				break;

			if(first)
				first = false;
			else {
				if(ic != ',') {
					err = "expected comma before next array element";
					return false;
				}

				ic = src.nextNonBlankChar();
				if(ic < 0) {
					err = "couldn't read from input";
					return false;
				}

				if(ic == ']')
					break;
			}

			src.undo();

			m::JSONElement elem;
			if(!g_m_json_parse(src, elem, err))
				return false;

			dst.addElement(elem);
		}

		return true;
	} else if(c == '\'' || c == '\"') {
		//String
		m::String val(2);
		if(!g_m_json_parseString(src, val, err, c))
			return false;

		dst = m::JSONElement(val);
		return true;
	} else if((c >= '0' && c <= '9') || c == '.' || c == '-') {
		//Number (FIXME: could be faster if parsed directly instead of using a string)
		bool hasDot = c == '.';
		m::String tmp(2);
		tmp += c;

		while(true) {
			int ic = src.nextCharRaw();
			if(ic < 0) {
				err = "couldn't read from input";
				return false;
			}

			c = static_cast<char>(ic);
			if(c == '.') {
				if(hasDot) {
					err = "invalid number format";
					return false;
				}

				hasDot = true;
				tmp += c;
			} else if(c >= '0' && c <= '9')
				tmp += c;
			else {
				src.undo();
				break;
			}
		}

		if(hasDot)
			dst = m::JSONElement(tmp.toDouble());
		else
			dst = m::JSONElement(tmp.toInteger());

		return true;
	} else if(c == 't') {
		//True
		if(src.expect("rue", 3)) {
			dst = m::JSONElement(true);
			return true;
		}
	} else if(c == 'f') {
		//False
		if(src.expect("alse", 4)) {
			dst = m::JSONElement(false);
			return true;
		}
	} else if(c == 'n') {
		//Null
		if(src.expect("ull", 3)) {
			dst = m::JSONElement();
			return true;
		}
	}

	err = "invalid token";
	return false;
}

bool m::json::parse(SSharedPtr<InputStream> src, JSONElement &dst, String &err)
{
	if(src.isNull())
		return false;

	BasicParser bp(src);
	String err2;
	if(g_m_json_parse(bp, dst, err2))
		return true;

	err.append("line ", 5);
	err += String::fromInteger(bp.line());
	err.append(", column ", 9);
	err += String::fromInteger(bp.column());
	err.append(": ", 2);
	err += err2;
	return false;
}

#define G_M_JSON_WRITECHR(out, chr) (out->write(reinterpret_cast<const uint8_t*>(chr), 1) == 1)

static bool g_m_json_writeStr(m::OutputStream *out, const char *str, int len)
{
	return m::IO::writeFully(out, reinterpret_cast<const uint8_t*>(str), len);
}

static bool g_m_json_serializeStr(m::OutputStream *out, const m::String &str)
{
	if(!G_M_JSON_WRITECHR(out, "\""))
		return false;

	for(int i = 0; i < str.length(); i++) {
		bool status;
		char c = str[i];

		if(c == '\\')
			status = g_m_json_writeStr(out, "\\\\", 2);
		else if(c == '\t')
			status = g_m_json_writeStr(out, "\\t", 2);
		else if(c == '\r')
			status = g_m_json_writeStr(out, "\\r", 2);
		else if(c == '\n')
			status = g_m_json_writeStr(out, "\\n", 2);
		else if(c == '\b')
			status = g_m_json_writeStr(out, "\\b", 2);
		else if(c == '\f')
			status = g_m_json_writeStr(out, "\\f", 2);
		else if(c == '\"')
			status = g_m_json_writeStr(out, "\\\"", 2);
		else
			status = G_M_JSON_WRITECHR(out, &c);

		if(!status)
			return false;
	}

	return G_M_JSON_WRITECHR(out, "\"");
}

bool g_m_json_serializeCompact(m::OutputStream *out, m::JSONElement &src)
{
	bool status;
	switch(src.type()) {
	case m::kJT_Boolean:
		if(src.asBool())
			status = g_m_json_writeStr(out, "true", 4);
		else
			status = g_m_json_writeStr(out, "false", 5);

		break;

	case m::kJT_Number:
	{
		m::String nbr(m::String::fromDouble(src.asDouble(), 8));
		status = g_m_json_writeStr(out, nbr.raw(), nbr.length());
		break;
	}

	case m::kJT_String:
		status = g_m_json_serializeStr(out, src.asString());
		break;

	case m::kJT_Array:
	{
		if(!G_M_JSON_WRITECHR(out, "["))
			return false;

		int asz = src.size();
		for(int i = 0; i < asz; i++) {
			if(i > 0 && !G_M_JSON_WRITECHR(out, ","))
				return false;

			if(!g_m_json_serializeCompact(out, src[i]))
				return false;
		}

		status = G_M_JSON_WRITECHR(out, "]");
		break;
	}

	case m::kJT_Object:
	{
		if(!G_M_JSON_WRITECHR(out, "{"))
			return false;

		int osz = src.size();
		for(int i = 0; i < osz; i++) {
			if(i > 0 && !G_M_JSON_WRITECHR(out, ","))
				return false;

			if(!g_m_json_serializeStr(out, src[i].name()))
				return false;

			if(!G_M_JSON_WRITECHR(out, ":") || !g_m_json_serializeCompact(out, src[i]))
				return false;
		}

		status = G_M_JSON_WRITECHR(out, "}");
		break;
	}

	default:
		status = g_m_json_writeStr(out, "null", 4);
		break;
	}

	return status;
}

bool m::json::serializeCompact(SSharedPtr<OutputStream> out, JSONElement &src)
{
	return !out.isNull() && g_m_json_serializeCompact(out.ptr(), src);
}

bool g_m_json_writeTabs(m::OutputStream *out, int tabs)
{
	for(int i = 0; i < tabs; i++) {
		if(!G_M_JSON_WRITECHR(out, "\t"))
			return false;
	}

	return true;
}

bool g_m_json_serializeHumanReadable(m::OutputStream *out, m::JSONElement &src, int tabs)
{
	if(tabs < 0)
		tabs = -tabs;
	else if(!g_m_json_writeTabs(out, tabs))
		return false;

	bool status;
	switch(src.type()) {
	case m::kJT_Boolean:
		if(src.asBool())
			status = g_m_json_writeStr(out, "true", 4);
		else
			status = g_m_json_writeStr(out, "false", 5);

		break;

	case m::kJT_Number:
	{
		m::String nbr(m::String::fromDouble(src.asDouble(), 8));
		status = g_m_json_writeStr(out, nbr.raw(), nbr.length());
		break;
	}

	case m::kJT_String:
		status = g_m_json_serializeStr(out, src.asString());
		break;

	case m::kJT_Array:
	{
		int asz = src.size();

		if(asz == 0)
			status = g_m_json_writeStr(out, "[]", 2);
		else {
			if(!g_m_json_writeStr(out, "[\n", 2))
				return false;

			for(int i = 0; i < asz; i++) {
				if(i > 0 && !g_m_json_writeStr(out, ",\n", 2))
					return false;

				if(!g_m_json_serializeHumanReadable(out, src[i], tabs + 1))
					return false;
			}

			if(!G_M_JSON_WRITECHR(out, "\n") || !g_m_json_writeTabs(out, tabs))
				return false;

			status = G_M_JSON_WRITECHR(out, "]");
		}

		break;
	}

	case m::kJT_Object:
	{
		int osz = src.size();

		if(osz == 0)
			status = g_m_json_writeStr(out, "{}", 2);
		else {
			if(!g_m_json_writeStr(out, "{\n", 2))
				return false;

			for(int i = 0; i < osz; i++) {
				if(i > 0 && !g_m_json_writeStr(out, ",\n", 2))
					return false;

				if(!g_m_json_writeTabs(out, tabs + 1) || !g_m_json_serializeStr(out, src[i].name()))
					return false;

				if(!g_m_json_writeStr(out, ": ", 2) || !g_m_json_serializeHumanReadable(out, src[i], -(tabs + 1)))
					return false;
			}

			if(!G_M_JSON_WRITECHR(out, "\n") || !g_m_json_writeTabs(out, tabs))
				return false;

			status = G_M_JSON_WRITECHR(out, "}");
		}

		break;
	}

	default:
		status = g_m_json_writeStr(out, "null", 4);
		break;
	}

	return status;
}

bool m::json::serializeHumanReadable(SSharedPtr<OutputStream> out, JSONElement &src)
{
	return !out.isNull() && g_m_json_serializeHumanReadable(out.ptr(), src, 0);
}

