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

#include "mgpcl/URL.h"

m::URLParseError m::URL::parse(const String &url)
{
	int protoPos = url.indexOf(':');
	if(protoPos <= 0)
		return kUPE_MissingProtocol;

	if(protoPos + 2 >= url.length() || url[protoPos + 1] != '/' || url[protoPos + 2] != '/')
		return kUPE_InvalidFormat;

	int hostPos = protoPos + 3;
	if(hostPos >= url.length())
		return kUPE_MissingHost;

	int portPos = url.indexOf(':', hostPos);
	int urlPos = url.indexOf('/', hostPos);
	bool hasPort = false;

	if(urlPos < 0)
		urlPos = url.length();

	if(portPos >= 0 && portPos <= urlPos) {
		if(portPos >= url.length()) //It's at the end
			return kUPE_InvalidPort;

		//We have a port, sir!
		uint32_t p = 0;
		for(int i = portPos + 1; i < urlPos; i++) {
			if(url[i] >= '0' && url[i] <= '9')
				p = p * 10 + static_cast<uint32_t>(url[i] - '0');
			else
				return kUPE_InvalidPort;
		}

		if(p >= 65536)
			return kUPE_InvalidPort;

		hasPort = true;
		m_port = p;
	}

	if(!checkStringRange(url, 0, protoPos, false))
		return kUPE_InvalidCharacter;

	if(!checkStringRange(url, hostPos, hasPort ? portPos : urlPos, true))
		return kUPE_InvalidCharacter;

	m_proto = url.substr(0, protoPos).lower(); //Set it to lowercase so we can find the corresponding port
	m_host = url.substr(hostPos, hasPort ? portPos : urlPos);
	m_location = url.substr(urlPos);

	if(!hasPort) {
		if(m_proto == "http")
			m_port = 80;
		else if(m_proto == "https")
			m_port = 443;
	}

	return kUPE_NoError;
}

m::URLParseError m::URL::parseRelative(const URL &src, const String &url)
{
	if(!src.isValid())
		return kUPE_SourceInvalid;

	if(url.isEmpty()) {
		*this = src;
		return kUPE_NoError;
	}

	if(url.length() >= 2 && url[0] == '/' && url[1] == '/') {
		//Protocol relative
		String tmp(src.m_proto.length() + 1 + url.length());
		tmp += src.m_proto;
		tmp += ':';
		tmp += url;

		return parse(tmp);
	}
	
	if(url[0] == '/') {
		//Relative to site root
		m_proto = src.m_proto;
		m_host = src.m_host;
		m_port = src.m_port;
		m_location = url;
		return kUPE_NoError;
	}

	int begin;
	for(begin = 0; begin < url.length(); begin++) {
		char c = url[begin];

		if((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && (c < '0' || c > '9') && c != '-' && c != '_' && c != '~')
			break;
	}

	if(begin + 2 < url.length() && url[begin] == ':' && url[begin + 1] == '/' && url[begin + 2] == '/')
		return parse(url); //ABSOLUTE, the caller lied to us :p

	//Relative to src's location, keep everything, append at the end
	m_proto = src.m_proto;
	m_host = src.m_host;
	m_port = src.m_port;

	if(!src.m_location.isEmpty() && src.m_location[src.m_location.length() - 1] != '/') {
		//Erase last file name from src.m_location
		//There MUST be a slash before the last character of src.m_location

		int slash = src.m_location.lastIndexOf('/');
		if(slash < 0)
			return kUPE_InvalidFormat; //As I said, this should NEVER happen, given the results of parse() and given the previous condition

		m_location = src.m_location.substr(0, slash + 1); //Include the slash; we'll need to add it anyway
	} else
		m_location = src.m_location;

	m_location += url;
	return kUPE_NoError;
}

bool m::URL::checkStringRange(const String &str, int a, int b, bool dotOk)
{
	while(a < b) {
		char c = str[a++];

		if(c == '.') {
			if(!dotOk)
				return false;
		} else if((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && (c < '0' || c > '9') && c != '-' && c != '_' && c != '~')
			return false;
	}

	return true;
}

m::String m::URL::encode(const String &u)
{
	String ret(u.length()); //Reserve at least u's length
	const char *hex = "0123456789ABCDEF";

	for(int i = 0; i < u.length(); i++) {
		char c = u[i];

		if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '~')
			ret += c;
		else {
			//Apply percent encoding
			ret += '%';
			ret += hex[(c & 0xF0) >> 4];
			ret += hex[c & 0x0F];
		}
	}

	return ret;
}

bool m::URL::hexDecode(char &c)
{
	if(c >= '0' && c <= '9')
		c -= '0';
	else if(c >= 'A' && c <= 'F')
		c = c - 'A' + 10;
	else if(c >= 'a' && c <= 'f')
		c = c - 'a' + 10;
	else
		return false;

	return true;
}


bool m::URL::decode(const String &u, String &dst)
{
	dst.clear();
	dst.reserve(u.length());

	for(int i = 0; i < u.length(); i++) {
		char c = u[i];

		if(c == '%') {
			if(i + 2 >= u.length())
				return false;

			char l = u[i + 1];
			char r = u[i + 2];

			if(!hexDecode(l) || !hexDecode(r))
				return false;

			char result = (l << 4) | r;
			dst += result;
			i += 2;
		} else
			dst += c;
	}

	return true;
}

m::String m::URL::toString() const
{
	bool putPort = true;
	if(m_proto == "http" && m_port == 80)
		putPort = false;
	else if(m_proto == "https" && m_port == 443)
		putPort = false;

	String ret(m_proto.length() + 3 + m_host.length() + (putPort ? 2 : 0) + m_location.length());
	ret += m_proto;
	ret.append("://", 3);
	ret += m_host;

	if(putPort) {
		ret += ':';
		ret += String::fromUInteger(static_cast<uint32_t>(m_port));
	}

	ret += m_location; //Location is either empty OR starts with a slash
	return ret;
}
