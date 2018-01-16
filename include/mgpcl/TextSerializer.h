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

#pragma once
#include "String.h"
#include "Enums.h"
#include "Math.h"
#include <cstdint>

namespace m
{

    class TextSerializerEndOfLine
    {
    };

    const TextSerializerEndOfLine eol;
    
    class TextSerializer
    {
    protected:
        virtual void tsWrite(const char *txt, int len) = 0;

    public:
        TextSerializer()
        {
            m_le = M_OS_LINEENDING;
            m_prec = 2;
        }

        TextSerializer(LineEnding le)
        {
            m_le = le;
            m_prec = 2;
        }

        virtual ~TextSerializer()
        {
        }

        TextSerializer &operator << (int32_t i)
        {
            String str(String::fromInteger(i));
            tsWrite(str.raw(), str.length());
            return *this;
        }

        TextSerializer &operator << (int16_t i)
        {
            String str(String::fromInteger(static_cast<int>(i)));
            tsWrite(str.raw(), str.length());
            return *this;
        }

        TextSerializer &operator << (char c)
        {
            tsWrite(&c, 1);
            return *this;
        }

        TextSerializer &operator << (double i)
        {
            String str(String::fromDouble(i, m_prec));
            tsWrite(str.raw(), str.length());
            return *this;
        }

        TextSerializer &operator << (float i)
        {
            String str(String::fromDouble(static_cast<double>(i), m_prec));
            tsWrite(str.raw(), str.length());
            return *this;
        }

        TextSerializer &operator << (uint32_t i)
        {
            String str(String::fromUInteger(i));
            tsWrite(str.raw(), str.length());
            return *this;
        }

        TextSerializer &operator << (uint16_t i)
        {
            String str(String::fromUInteger(i));
            tsWrite(str.raw(), str.length());
            return *this;
        }

        TextSerializer &operator << (uint8_t i)
        {
            char str[4];
            str[0] = '0';
            str[1] = 'x';
            str[2] = (i & 0xF0) >> 4;
            str[3] = i & 0x0F;

            if(str[2] < 10)
                str[2] = '0' + str[2];
            else
                str[2] = 'a' + (str[2] - 10);

            if(str[3] < 10)
                str[3] = '0' + str[3];
            else
                str[3] = 'a' + (str[3] - 10);

            tsWrite(str, 4);
            return *this;
        }

        TextSerializer &operator << (const String &str)
        {
            tsWrite(str.raw(), str.length());
            return *this;
        }

        TextSerializer &operator << (const char *str)
        {
            int len = 0;
            while(str[len] != 0)
                len++;

            tsWrite(str, len);
            return *this;
        }

        TextSerializer &operator << (const void *ptr)
        {
            String str(String::fromPointer(ptr));
            tsWrite(str.raw(), str.length());
            return *this;
        }

        TextSerializer &operator << (TextSerializerEndOfLine tseol)
        {
            switch(m_le) {
            case LineEnding::CRLF:
                tsWrite("\r\n", 2);
                break;

            case LineEnding::LFOnly:
                tsWrite("\n", 1);
                break;

            case LineEnding::CROnly:
                tsWrite("\r", 1);
                break;

            default:
                break;
            }

            return *this;
        }

        void setPrecision(int prec)
        {
            m_prec = prec;
        }

        int precision() const
        {
            return m_prec;
        }

        void setLineEnding(LineEnding le)
        {
            m_le = le;
        }

        LineEnding lineEnding() const
        {
            return m_le;
        }

    protected:
        int m_prec;
        LineEnding m_le;
    };

    class TextDeserializer
    {
    protected:
        virtual bool tsRead(char *dst, int len) = 0;
        int tsReadChr()
        {
            if(m_hasUndo) {
                m_hasUndo = false;
                return m_undo;
            }

            char ret;
            if(!tsRead(&ret, 1)) {
                m_reachedEOF = true;
                return -1;
            }

            return static_cast<int>(ret);
        }

    public:
        TextDeserializer()
        {
            m_hasUndo = false;
            m_undo = 0;
            m_skipWS = true;
            m_reachedEOF = false;
        }

        virtual ~TextDeserializer()
        {
        }

        void skipWS()
        {
            char chr;
            int chr2;

            do {
                chr2 = tsReadChr();
                if(chr2 < 0)
                    return;

                chr = static_cast<char>(chr2);
            } while(chr == ' ' || chr == '\t' || chr == '\r' || chr == '\n');

            m_hasUndo = true;
            m_undo = chr;
        }

        TextDeserializer &operator >> (char &dst)
        {
            if(m_skipWS)
                skipWS();

            dst = static_cast<char>(tsReadChr());
            return *this;
        }

        uint8_t readByte()
        {
            uint8_t ret;
            if(m_hasUndo) {
                ret = static_cast<uint8_t>(m_undo);
                m_hasUndo = false;
            } else {
                if(!tsRead(reinterpret_cast<char*>(&ret), 1))
                    m_reachedEOF = true;
            }

            return ret;
        }

        TextDeserializer &operator >> (uint32_t &dst)
        {
            if(m_skipWS)
                skipWS();

            dst = parseUInt();
            return *this;
        }

        TextDeserializer &operator >> (int32_t &dst)
        {
            if(m_skipWS)
                skipWS();

            int chr2 = tsReadChr();
            if(chr2 < 0)
                return *this;

            char chr = static_cast<char>(chr2);
            if(chr != '-') {
                m_hasUndo = true;
                m_undo = chr;
            }

            uint32_t ret = parseUInt();
            if(chr == '-')
                dst = -static_cast<int>(ret);
            else
                dst = static_cast<int>(ret);

            return *this;
        }

        TextDeserializer &operator >> (int16_t &dst)
        {
            int32_t ret;
            *this >> ret;
            dst = static_cast<int16_t>(ret);

            return *this;
        }

        TextDeserializer &operator >> (uint16_t &dst)
        {
            uint32_t ret;
            *this >> ret;
            dst = static_cast<uint16_t>(ret);

            return *this;
        }

        TextDeserializer &operator >> (uint8_t &dst)
        {
            uint32_t ret;
            *this >> ret;
            dst = static_cast<uint8_t>(ret);

            return *this;
        }

        TextDeserializer &operator >> (double &dst)
        {
            if(m_skipWS)
                skipWS();

            dst = 0.0;
            int chr2 = tsReadChr();
            bool neg;
            char chr;

            if(chr2 < 0)
                return *this;

            chr = static_cast<char>(chr2);

            if(dst == '-') {
                neg = true;

                chr2 = tsReadChr();
                if(chr2 < 0)
                    return *this;

                chr = static_cast<char>(chr2);
            } else
                neg = false;

            while(chr >= '0' && chr <= '9') {
                dst = dst * 10.0 + static_cast<double>(chr - '0');
                chr2 = tsReadChr();

                if(chr2 < 0)
                    return *this;

                chr = static_cast<char>(chr2);
            }

            if(chr == '.') {
                double div = 0.1;
                chr2 = tsReadChr();

                if(chr2 < 0)
                    return *this;

                chr = static_cast<char>(chr2);
                while(chr >= '0' && chr <= '9') {
                    dst = dst + static_cast<double>(chr - '0') * div;
                    div *= 0.1;
                    chr2 = tsReadChr();

                    if(chr2 < 0)
                        return *this;

                    chr = static_cast<char>(chr2);
                }
            }

            if(chr == 'e' || chr == 'E') {
                //Exponent?
                bool negExp;
                uint32_t exp = 0;
                chr2 = tsReadChr();

                if(chr2 < 0)
                    return *this;

                chr = static_cast<char>(chr2);

                if(chr == '-') {
                    negExp = true;
                    chr2 = tsReadChr();

                    if(chr2 < 0)
                        return *this;

                    chr = static_cast<char>(chr2);
                } else
                    negExp = false;

                while(chr >= '0' && chr <= '9') {
                    exp = exp * 10 + static_cast<uint32_t>(chr - '0');
                    chr2 = tsReadChr();

                    if(chr2 < 0)
                        return *this;

                    chr = static_cast<char>(chr2);
                }

                if(negExp)
                    dst /= math::pow(10.0, static_cast<double>(exp));
                else
                    dst *= math::pow(10.0, static_cast<double>(exp));
            }

            if(neg)
                dst = -dst;

            m_hasUndo = true;
            m_undo = chr;
            return *this;
        }

        TextDeserializer &operator >> (float &dst)
        {
            double ret;
            *this >> ret;

            dst = static_cast<float>(ret);
            return *this;
        }

        TextDeserializer &operator >> (String &str)
        {
            if(m_skipWS)
                skipWS();

            int chr2 = tsReadChr();
            if(chr2 < 0)
                return *this;

            char chr = static_cast<char>(chr2);
            while(chr != ' ' && chr != '\t' && chr != '\r' && chr != '\n') {
                str += chr;
                chr2 = tsReadChr();

                if(chr2 < 0)
                    return *this;

                chr = static_cast<char>(chr2);
            }

            m_hasUndo = true;
            m_undo = chr;
            return *this;
        }

        TextDeserializer &readLine(String &str)
        {
            int chr2 = tsReadChr();
            if(chr2 < 0)
                return *this;

            char chr = static_cast<char>(chr2);
            while(chr != '\r' && chr != '\n') {
                str += chr;
                chr2 = tsReadChr();

                if(chr2 < 0)
                    return *this;

                chr = static_cast<char>(chr2);
            }

            if(chr == '\r') {
                chr2 = tsReadChr();
                if(chr2 < 0)
                    return *this;

                chr = static_cast<char>(chr2);
                if(chr != '\n') {
                    m_hasUndo = true;
                    m_undo = chr;
                }
            }

            return *this;
        }

        bool hasReachedEOF() const
        {
            return m_reachedEOF;
        }

        bool operator ! () const
        {
            return m_reachedEOF;
        }

    private:
        uint32_t parseUInt()
        {
            uint32_t base = 10;
            char maxChar = '9';
            uint32_t dst = 0;

            int chr2 = tsReadChr();
            if(chr2 < 0)
                return dst;

            char chr = static_cast<char>(chr2);
            if(chr == '0') {
                chr2 = tsReadChr();
                if(chr2 < 0)
                    return dst;

                chr = static_cast<char>(chr2);

                if(chr == 'x') {
                    base = 16;
                    maxChar = 'f';

                    chr2 = tsReadChr();
                    if(chr2 < 0)
                        return dst;

                    chr = static_cast<char>(chr2);
                } else if(chr == 'b') {
                    base = 2;
                    maxChar = '1';

                    chr2 = tsReadChr();
                    if(chr2 < 0)
                        return dst;

                    chr = static_cast<char>(chr2);
                }
            }

            if(maxChar <= '9') {
                while(chr >= '0' && chr <= maxChar) {
                    dst = dst * base + static_cast<uint32_t>(chr - '0');
                    chr2 = tsReadChr();

                    if(chr2 < 0)
                        return dst;

                    chr = static_cast<char>(chr2);
                }
            } else {
                //Alphanumeric
                char chrl = static_cast<char>(tolower(chr));

                while((chrl >= '0' && chrl <= '9') || (chrl >= 'a' && chrl <= maxChar)) {
                    if(chrl >= '0' && chrl <= '9')
                        dst = dst * base + static_cast<uint32_t>(chrl - '0');
                    else
                        dst = dst * base + static_cast<uint32_t>(chrl - 'a') + 10;

                    chr2 = tsReadChr();
                    if(chr2 < 0)
                        return dst;

                    chrl = static_cast<char>(tolower(chr2));
                }
            }

            m_hasUndo = true;
            m_undo = chr;
            return dst;
        }

        bool m_hasUndo;
        char m_undo;
        bool m_skipWS;
        bool m_reachedEOF;
    };

}
