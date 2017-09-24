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

#include "mgpcl/IPv4Address.h"
#include "mgpcl/Mem.h"

m::IPv4Address::IPv4Address()
{
    Mem::zero(&m_sain, sizeof(struct sockaddr_in));
    m_sain.sin_family = AF_INET;
}

m::IPv4Address::IPv4Address(uint16_t port)
{
    Mem::zero(&m_sain, sizeof(struct sockaddr_in));
    m_sain.sin_family = AF_INET;
    m_sain.sin_port = ((port & 0xFF00) >> 8) | ((port & 0x00FF) << 8);

    if(INADDR_ANY) //Hopefully this 'if' will be removed by the compiler, because INADDR_ANY is a constant
        m_sain.sin_addr.s_addr = INADDR_ANY;
}

m::IPv4Address::IPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port)
{
    Mem::zero(&m_sain, sizeof(struct sockaddr_in));
    m_sain.sin_family = AF_INET;
    m_sain.sin_port = ((port & 0xFF00) >> 8) | ((port & 0x00FF) << 8);
    m_sain.sin_addr.s_addr = static_cast<uint32_t>(a) | (static_cast<uint32_t>(b) << 8) | (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24);
}

m::DNSResolveError m::IPv4Address::resolve(const String &str, uint16_t port)
{
    struct addrinfo hints, *result;
    Mem::zero(&hints, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;

    int ret = getaddrinfo(str.raw(), nullptr, &hints, &result);

    if(ret != 0) {
        switch(ret) {
#ifdef EAI_ADDRFAMILY
        case EAI_ADDRFAMILY:
            return kRE_NonIPv4Host;
#endif

        case EAI_AGAIN:
            return kRE_TryLater;

        case EAI_FAIL:
            return kRE_DNSFailure;

        case EAI_MEMORY:
            return kRE_OutOfMemory;

#if EAI_NONAME != EAI_NODATA
        case EAI_NONAME:
#endif

        case EAI_NODATA:
            return kRE_UnknownHost;

#ifdef EAI_SYSTEM
        case EAI_SYSTEM:
            return kRE_SystemError;
#endif

        default:
            return kRE_UnknownError;
        }
    }

    if(result == nullptr)
        return kRE_UnknownError;

    if(result->ai_family != AF_INET || result->ai_addrlen != sizeof(struct sockaddr_in) || result->ai_addr == nullptr || result->ai_addr->sa_family != AF_INET) {
        freeaddrinfo(result);
        return kRE_UnknownError;
    }

    Mem::copy(&m_sain, result->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(result);

    m_sain.sin_port = ((port & 0xFF00) >> 8) | ((port & 0x00FF) << 8);
    return kRE_NoError;
}

m::AddressFormatError m::IPv4Address::parse(const String &str)
{
    int num;
    int pos = 0;

    if(str.length() < 1)
        return kAFE_InvalidFormat;

    if(str[0] == '*')
        m_sain.sin_addr.s_addr = INADDR_ANY;
    else {
        for(int f = 0; f < 4; f++) {
            num = 0;
            while(pos < str.length() && str[pos] >= '0' && str[pos] <= '9') {
                num = num * 10 + static_cast<int>(str[pos] - '0');
                pos++;
            }

            if(f < 3) {
                if(pos >= str.length() || str[pos] != '.')
                    return kAFE_InvalidFormat;

                pos++; //Skip dot
            }

            if(num < 0 || num >= 256)
                return kAFE_InvalidAddressNumber;

            reinterpret_cast<uint8_t*>(&m_sain.sin_addr.s_addr)[f] = static_cast<uint8_t>(num);
        }
    }

    if(pos >= str.length())
        return kAFE_MissingPort;
    else if(str[pos] != ':')
        return kAFE_InvalidFormat;

    pos++; //Skip :
    num = 0;

    if(pos >= str.length())
        return kAFE_InvalidFormat; //Missing numbers after :

    do {
        if(str[pos] >= '0' && str[pos] <= '9') {
            num = num * 10 + static_cast<int>(str[pos] - '0');
            pos++;
        } else
            return kAFE_InvalidFormat;
    } while(pos < str.length());

    if(num < 0 || num >= 65536)
        return kAFE_InvalidPortNumber;

    m_sain.sin_port = (static_cast<uint16_t>(num & 0x0000FF00) >> 8) | (static_cast<uint16_t>(num & 0x000000FF) << 8);
    return kAFE_NoError;
}

m::AddressFormatError m::IPv4Address::parse(const String &str, uint16_t defPort)
{
    AddressFormatError ret = parse(str);
    if(ret == kAFE_MissingPort) {
        m_sain.sin_port = ((defPort & 0xFF00) >> 8) | ((defPort & 0x00FF) << 8);
        return kAFE_NoError;
    }

    return ret;
}

m::String m::IPv4Address::toString(bool withPort) const
{
    if(m_sain.sin_addr.s_addr == INADDR_ANY) {
        String ret(withPort ? 3 : 1);
        ret += '*';

        if(withPort) {
            ret += ':';
            ret += String::fromUInteger(static_cast<uint32_t>(((m_sain.sin_port & 0xFF00) >> 8) | ((m_sain.sin_port & 0x00FF) << 8)));
        }

        return ret;
    } else {
        String ret(withPort ? 9 : 7);
        for(int i = 0; i < 4; i++) {
            if(i != 0)
                ret += '.';

            ret += String::fromUInteger(static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(&m_sain.sin_addr.s_addr)[i]));
        }

        if(withPort) {
            ret += ':';
            ret += String::fromUInteger(static_cast<uint32_t>(((m_sain.sin_port & 0xFF00) >> 8) | ((m_sain.sin_port & 0x00FF) << 8)));
        }

        return ret;
    }
}

