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

#define M_UTIL_DECLARE

#include <cstdint>
#include "mgpcl/Util.h"
#include "mgpcl/String.h"

bool m::IO::readFully(InputStream *dst, uint8_t *data, int len)
{
    while(len > 0) {
        int rd = dst->read(data, len);
        if(rd <= 0)
            return false;

        data += rd;
        len -= rd;
    }

    return true;
}

bool m::IO::writeFully(OutputStream *dst, const uint8_t *data, int len)
{
    while(len > 0) {
        int rd = dst->write(data, len);
        if(rd <= 0)
            return false;

        data += rd;
        len -= rd;
    }

    return true;
}

bool m::IO::transfer(OutputStream *dst, InputStream *src, int bufSz)
{
    uint8_t *buf = new uint8_t[bufSz];
    bool keepGoing = true;
    bool ret = false;

    while(keepGoing) {
        int rd = src->read(buf, bufSz);

        if(rd > 0)
            writeFully(dst, buf, rd);
        else {
            keepGoing = false;
            ret = rd == 0;
        }
    }

    delete[] buf;
    return ret;
}

bool m::IO::writeLine(const TString<char> &str, OutputStream *dst, LineEnding le)
{
    const char *data = str.raw();
    int sz = str.length();

    while(sz > 0) {
        int written = dst->write(reinterpret_cast<const uint8_t*>(data), sz);
        if(written <= 0)
            return false;

        data += written;
        sz -= written;
    }

    switch(le) {
    case LineEnding::CRLF:
        sz = dst->write(reinterpret_cast<const uint8_t*>("\r\n"), 2);

        if(sz >= 2)
            return true;
        else if(sz <= 0)
            return false; //Otherwise it returned 1, in which case we don't break to write the \n..

    case LineEnding::LFOnly:
        return dst->write(reinterpret_cast<const uint8_t*>("\n"), 1) == 1;

    case LineEnding::CROnly:
        return dst->write(reinterpret_cast<const uint8_t*>("\r"), 1) == 1;

    default:
        return false;
    }
}

bool m::parseArgs(const String &str, List<String> &dst, bool strict)
{
    String toAdd;
    bool inQuote = false;
    char lastChar = 0;

    int lastPos = 0;
    while(lastPos < str.length() && (str[lastPos] == ' ' || str[lastPos] == '\t')) //trim left
        lastPos++;

    int i = lastPos;
    while(i < str.length()) {
        char c = str[i];
        bool escaped = false;

        if(inQuote) {
            if(lastChar == '\\') {
                //Escape c
                if(c == '\\') {
                    toAdd += str.substr(lastPos, i);
                    lastPos = ++i;
                    escaped = true;
                } else if(c == '\"') {
                    toAdd += str.substr(lastPos, i - 1); //Don't include the backslash
                    lastPos = i++;
                } //Ignore and keep going
            } else if(c == '\"') {
                toAdd += str.substr(lastPos, i);
                inQuote = false;
                lastPos = ++i;
            } else
                i++;
        } else if(c == '\"') {
            //Enter quoted argument

            if(lastChar == '\\') {
                toAdd += str.substr(lastPos, i - 1);
                toAdd += '\"';
            } else {
                if(lastChar == '\"')
                    toAdd += '\"';

                toAdd += str.substr(lastPos, i);
                inQuote = true;
            }

            lastPos = ++i;
        } else if(c == ' ' || c == '\t') {
            toAdd += str.substr(lastPos, i);
            dst.add(toAdd);
            toAdd.cleanup();

            while(i < str.length() && (str[i] == ' ' || str[i] == '\t')) //trim left
                i++;

            lastPos = i;
        } else
            i++;

        lastChar = escaped ? 0 : c;
    }

    int end = str.length();
    if(end >= 1 && str[end - 1] == '\"')
        end--;

    toAdd += str.substr(lastPos, end);
    dst.add(toAdd);

    return !strict || !inQuote;
}

static const char *g_units[] = { " KiB", " MiB", " GiB", " TiB", nullptr };

void m::makeSizeString(uint64_t sz, TString<char> &dst)
{
    if(sz < 1024) {
        dst += TString<char>::fromUInteger(static_cast<uint32_t>(sz));
        dst.append(" bytes", sz == 1 ? 5 : 6);
        return;
    }

    uint64_t rest;
    int idx = 0;

    do {
        rest = sz % 1024;
        sz /= 1024;
        idx++;
    } while(sz >= 1024 && g_units[idx] != nullptr);

    dst += TString<char>::fromUInteger(static_cast<uint32_t>(sz));

    if(rest != 0) {
        dst += '.';

        double d = static_cast<double>(rest) / 1024.0;
        d *= 100.0;

        dst += TString<char>::fromUInteger(static_cast<uint32_t>(d));
    }

    dst.append(g_units[idx - 1], 4);
}

static const char *g_hex = "0123456789abcdef";

void m::hexString(const uint8_t *data, uint32_t sz, TString<char> &dst)
{
    dst.reserve(dst.length() + static_cast<int>(sz * 2));

    for(uint32_t i = 0; i < sz; i++) {
        uint8_t left = (data[i] & 0xF0) >> 4;
        uint8_t right = data[i] & 0x0F;

        dst += g_hex[left];
        dst += g_hex[right];
    }
}

static const char *g_b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void m::base64Encode(const uint8_t *data, uint32_t sz, TString<char> &dst)
{
    const uint32_t mod = sz % 3;
    uint32_t expected = sz * 4;
    expected /= 3;

    if(mod == 2)
        expected += 2;
    else if(mod == 1)
        expected += 3;

    dst.reserve(dst.length() + expected);

    uint32_t i;
    for(i = 0; i + 2 < sz; i += 3) {
        uint32_t pack;
        pack  = static_cast<uint32_t>(data[i + 0]) << 16U;
        pack |= static_cast<uint32_t>(data[i + 1]) << 8U;
        pack |= static_cast<uint32_t>(data[i + 2]) << 0U;

        uint8_t a = static_cast<uint8_t>((pack & 0x00FC0000U) >> 18U);
        uint8_t b = static_cast<uint8_t>((pack & 0x0003F000U) >> 12U);
        uint8_t c = static_cast<uint8_t>((pack & 0x00000FC0U) >>  6U);
        uint8_t d = static_cast<uint8_t>((pack & 0x0000003FU) >>  0U);

        char tmp[4] = { g_b64[a], g_b64[b], g_b64[c], g_b64[d] };
        dst.append(tmp, 4);
    }

    if(mod == 2) {
        uint32_t pack;
        pack  = static_cast<uint32_t>(data[i + 0]) << 10U;
        pack |= static_cast<uint32_t>(data[i + 1]) <<  2U;

        uint8_t a = static_cast<uint8_t>((pack & 0x0003F000U) >> 12U);
        uint8_t b = static_cast<uint8_t>((pack & 0x00000FC0U) >>  6U);
        uint8_t c = static_cast<uint8_t>((pack & 0x0000003FU) >>  0U);

        char tmp[4] = { g_b64[a], g_b64[b], g_b64[c], '=' };
        dst.append(tmp, 4);
    } else if(mod == 1) {
        uint32_t pack;
        pack = static_cast<uint32_t>(data[i + 0]) << 4U;

        uint8_t a = static_cast<uint8_t>((pack & 0x00000FC0U) >> 6U);
        uint8_t b = static_cast<uint8_t>((pack & 0x0000003FU) >> 0U);

        char tmp[4] = { g_b64[a], g_b64[b], '=', '=' };
        dst.append(tmp, 4);
    }
}

bool m::base64Decode(const char *data, uint8_t *dst, uint32_t &sz, int dataLen)
{
    if(dataLen < 0)
        dataLen = static_cast<int>(strlen(data));

    if(dataLen % 4 != 0)
        return false; //Invalid input data

    uint32_t numEquals = 0;
    for(int i = dataLen - 1; i >= 0 && data[i] == '='; i--)
        numEquals++;

    if(numEquals > 2)
        return false; //Invalid input data (again!)

    uint32_t estimated = static_cast<uint32_t>(dataLen) * 3;
    estimated /= 4;
    estimated -= numEquals;

    if(dst == nullptr) {
        //Query minimal buffer length
        sz = estimated;
        return true;
    }

    if(sz < estimated)
        return false; //Output buffer too small

    uint32_t pos = 0;
    uint32_t bits = 0;
    uint32_t shift = 18;

    for(int i = 0; i < dataLen; i++) {
        char chr = data[i];
        uint8_t byte;

        if(chr >= 'A' && chr <= 'Z')
            byte = static_cast<uint8_t>(chr - 'A');
        else if(chr >= 'a' && chr <= 'z')
            byte = static_cast<uint8_t>(chr - 'a') + 26;
        else if(chr >= '0' && chr <= '9')
            byte = static_cast<uint8_t>(chr - '0') + 52;
        else if(chr == '+')
            byte = 62;
        else if(chr == '/')
            byte = 63;
        else if(chr == '=')
            break;
        else
            return false;

        if(shift == 0) {
            bits |= byte;
            dst[pos++] = static_cast<uint8_t>((bits & 0x00FF0000U) >> 16U);
            dst[pos++] = static_cast<uint8_t>((bits & 0x0000FF00U) >> 8U);
            dst[pos++] = static_cast<uint8_t>((bits & 0x000000FFU) >> 0U);

            bits = 0;
            shift = 18;
        } else {
            bits |= byte << shift;
            shift -= 6;
        }
    }

    if(numEquals == 1) {
        //Two more bytes
        dst[pos++] = static_cast<uint8_t>((bits & 0x00FF0000U) >> 16U);
        dst[pos++] = static_cast<uint8_t>((bits & 0x0000FF00U) >> 8U);
    } else if(numEquals == 2) {
        //Only one more
        dst[pos++] = static_cast<uint8_t>((bits & 0x00FF0000U) >> 16U);
    }

    sz = pos;
    return true;
}

bool m::base64Decode(const TString<char> &str, uint8_t *dst, uint32_t &sz)
{
    return base64Decode(str.raw(), dst, sz, str.length());
}

uint32_t m::unHexString(const TString<char> &src, uint8_t *dst, uint32_t bufSz)
{
    if((src.length() & 1) != 0)
        return 0;

    uint32_t sz = static_cast<uint32_t>(src.length()) >> 1;
    if(sz > bufSz)
        sz = bufSz;

    for(uint32_t i = 0; i < sz; i++) {
        uint8_t a = hexVal(src[static_cast<int>(i << 1)]);
        uint8_t b = hexVal(src[static_cast<int>((i << 1) | 1)]);

        if(a == 0xFF || b == 0xFF)
            return 0; //Found bad character in string

        dst[i] = (a << 4) | b;
    }

    return sz;
}

#include "mgpcl/CRC32_Poly.h"

uint32_t m::crc32(const uint8_t *data, uint32_t len, uint32_t crc)
{
    while(len > 0) {
        crc = g_crcTable[*data ^ static_cast<uint8_t>((crc >> 24U) & 0xFFU)] ^ (crc << 8U);
        data++;
        len--;
    }

    return crc;
}
