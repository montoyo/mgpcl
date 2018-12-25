/* Copyright (C) 2018 BARBOTIN Nicolas
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

#define HTTP_COMMONS_C
#include "mgpcl/HTTPCommons.h"
#include "mgpcl/Util.h"

static char g_hexChars[] = "0123456789ABCDEF";
static uint8_t g_charWhitelist[256];

enum WhiteListFlags
{
    kWLF_NonReserved = 1,
    kWLF_SmartEncodeIgnored = 2,
    kWLF_HexChar = 4
};

class CharWhitelistInitializer
{
public:
    CharWhitelistInitializer()
    {
        m::mem::zero(g_charWhitelist, 256);
        for(int i = 0; i < 6; i++) {
            g_charWhitelist['A' + i] = kWLF_NonReserved | kWLF_SmartEncodeIgnored | kWLF_HexChar;
            g_charWhitelist['a' + i] = kWLF_NonReserved | kWLF_SmartEncodeIgnored | kWLF_HexChar;
        }

        for(int i = 6; i < 26; i++) {
            g_charWhitelist['A' + i] = kWLF_NonReserved | kWLF_SmartEncodeIgnored;
            g_charWhitelist['a' + i] = kWLF_NonReserved | kWLF_SmartEncodeIgnored;
        }

        for(int i = 0; i < 10; i++)
            g_charWhitelist['0' + i] = kWLF_NonReserved | kWLF_SmartEncodeIgnored | kWLF_HexChar;

        g_charWhitelist['-'] = kWLF_NonReserved | kWLF_SmartEncodeIgnored;
        g_charWhitelist['_'] = kWLF_NonReserved | kWLF_SmartEncodeIgnored;
        g_charWhitelist['.'] = kWLF_NonReserved | kWLF_SmartEncodeIgnored;
        g_charWhitelist['~'] = kWLF_NonReserved | kWLF_SmartEncodeIgnored;

        g_charWhitelist['/'] = kWLF_SmartEncodeIgnored;
        g_charWhitelist['%'] = kWLF_SmartEncodeIgnored;
        g_charWhitelist['?'] = kWLF_SmartEncodeIgnored;
        g_charWhitelist['='] = kWLF_SmartEncodeIgnored;
        g_charWhitelist['&'] = kWLF_SmartEncodeIgnored;
        g_charWhitelist['#'] = kWLF_SmartEncodeIgnored;
        g_charWhitelist['*'] = kWLF_SmartEncodeIgnored; //THIS MIGHT CAUSE ISSUE
    }
};

static CharWhitelistInitializer g_charWhitelistInitializer;

m::String m::http::encodeURIComponent(const String &cmp)
{
    String ret(cmp.length());
    for(int i = 0; i < cmp.length(); i++) {
        char chr = cmp[i];
        uint8_t uchr = static_cast<uint8_t>(chr);

        if(g_charWhitelist[uchr] & kWLF_NonReserved)
            ret += chr;
        else {
            ret += '%';
            ret += g_hexChars[(uchr & 0xF0) >> 4];
            ret += g_hexChars[uchr & 0x0F];
        }
    }

    return ret;
}

m::String m::http::decodeURIComponent(const String &cmp)
{
    String ret(cmp.length());
    for(int i = 0; i < cmp.length(); i++) {
        char chr = cmp[i];

        if(chr == '%' && i + 2 < cmp.length() && (g_charWhitelist[cmp.byte(i + 1)] & kWLF_HexChar) != 0 && (g_charWhitelist[cmp.byte(i + 2)] & kWLF_HexChar) != 0) {
            uint8_t uchr = (hexVal(cmp[i + 1]) << 4) | hexVal(cmp[i + 2]);
            ret += static_cast<char>(uchr);
        } else
            ret += static_cast<char>(chr);
    }

    return ret;
}

m::String m::http::smartEncodePathname(const String &pathname)
{
    String ret(pathname.length());
    for(int i = 0; i < pathname.length(); i++) {
        char chr = pathname[i];
        uint8_t uchr = static_cast<uint8_t>(chr);

        if(g_charWhitelist[uchr] & kWLF_SmartEncodeIgnored)
            ret += chr;
        else {
            ret += '%';
            ret += g_hexChars[(uchr & 0xF0) >> 4];
            ret += g_hexChars[uchr & 0x0F];
        }
    }

    return ret;
}
