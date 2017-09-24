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
#include "Enums.h"
#include "String.h"
#include <cstdint>

namespace m
{

    //I assume your system is using little endian; because well...
    //That's what MGPCL was made for!

    class DataSerializer
    {
    protected:
        virtual void dsWrite(const uint8_t *data, int sz) = 0;

    private:
        template<typename T> DataSerializer &_dsWriteT(T val)
        {
            uint8_t *src = reinterpret_cast<uint8_t*>(&val);

            if(m_ed == Endianness::Little)
                dsWrite(src, sizeof(T));
            else {
                uint8_t dst[sizeof(T)];
                for(uint8_t i = 0; i < sizeof(T); i++)
                    dst[i] = src[sizeof(T) - i - 1];

                dsWrite(dst, sizeof(T));
            }

            return *this;
        }

    public:
        //Constructor
        DataSerializer()
        {
            m_ed = Endianness::Little;
            m_stringMode = StringSerialization::UShortLenAndContent;
        }

        DataSerializer(Endianness ed)
        {
            m_ed = ed;
            m_stringMode = StringSerialization::UShortLenAndContent;
        }

        virtual ~DataSerializer()
        {
        }

        //Write operators
        DataSerializer &operator << (char c)
        {
            dsWrite(reinterpret_cast<uint8_t*>(&c), 1);
            return *this;
        }

        DataSerializer &operator << (int16_t data)
        {
            if(m_ed == Endianness::Little)
                dsWrite(reinterpret_cast<uint8_t*>(&data), 2);
            else {
                int16_t tmp = ((data & 0x00FF) << 8) | ((data & 0xFF00) >> 8);
                dsWrite(reinterpret_cast<uint8_t*>(&tmp), 2);
            }

            return *this;
        }

        DataSerializer &operator << (int32_t data)
        {
            if(m_ed == Endianness::Little)
                dsWrite(reinterpret_cast<uint8_t*>(&data), 4);
            else {
                int32_t tmp = ((data & 0x000000FF) << 24) | ((data & 0x0000FF00) << 8) | ((data & 0x00FF0000) >> 8) | ((data & 0xFF000000) >> 24);
                dsWrite(reinterpret_cast<uint8_t*>(&tmp), 4);
            }

            return *this;
        }

        DataSerializer &operator << (int64_t data)
        {
            return _dsWriteT<int64_t>(data);
        }

        DataSerializer &operator << (uint8_t b)
        {
            dsWrite(&b, 1);
            return *this;
        }

        DataSerializer &operator << (uint16_t data)
        {
            return *this << *reinterpret_cast<int16_t*>(&data);
        }

        DataSerializer &operator << (uint32_t data)
        {
            return *this << *reinterpret_cast<int32_t*>(&data);
        }

        DataSerializer &operator << (uint64_t data)
        {
            return _dsWriteT<uint64_t>(data);
        }

        DataSerializer &operator << (bool data)
        {
            uint8_t tmp = data ? 1 : 0;
            dsWrite(&tmp, 1);
            return *this;
        }

        DataSerializer &operator << (float data)
        {
            return *this << *reinterpret_cast<int32_t*>(&data);
        }

        DataSerializer &operator << (double data)
        {
            return _dsWriteT<double>(data);
        }

        DataSerializer &operator << (const String &str)
        {
            switch(m_stringMode) {
            case StringSerialization::AppendNullByte:
                dsWrite(reinterpret_cast<const uint8_t*>(str.raw()), str.length() + 1);
                break;

            case StringSerialization::ByteLenAndContent:
            {
                uint8_t len = static_cast<uint8_t>(str.length());
                dsWrite(&len, 1);
                dsWrite(reinterpret_cast<const uint8_t*>(str.raw()), str.length());
            }

            break;

            case StringSerialization::UShortLenAndContent:
                *this << static_cast<uint16_t>(str.length());
                dsWrite(reinterpret_cast<const uint8_t*>(str.raw()), str.length());
                break;
            }

            return *this;
        }

        //Write methods
        DataSerializer &writeChar(char c)
        {
            return *this << c;
        }

        DataSerializer &writeShort(int16_t c)
        {
            return *this << c;
        }

        DataSerializer &writeInt(int32_t c)
        {
            return *this << c;
        }

        DataSerializer &writeInt64(int64_t c)
        {
            return *this << c;
        }

        DataSerializer &writeByte(uint8_t c)
        {
            return *this << c;
        }

        DataSerializer &writeUShort(uint16_t c)
        {
            return *this << c;
        }

        DataSerializer &writeUInt(uint32_t c)
        {
            return *this << c;
        }

        DataSerializer &writeUInt64(uint64_t c)
        {
            return *this << c;
        }

        DataSerializer &writeBool(bool b)
        {
            return *this << b;
        }

        DataSerializer &writeFloat(float c)
        {
            return *this << c;
        }

        DataSerializer &writeDouble(double c)
        {
            return *this << c;
        }

        DataSerializer &writeString(const String &str)
        {
            return *this << str;
        }

        //Getters & setters
        Endianness endianness() const
        {
            return m_ed;
        }

        void setEndianness(Endianness ed)
        {
            m_ed = ed;
        }

        StringSerialization stringMode() const
        {
            return m_stringMode;
        }

        void setStringMode(StringSerialization stringMode)
        {
            m_stringMode = stringMode;
        }

    protected:
        Endianness m_ed;
        StringSerialization m_stringMode;
    };

    class DataDeserializer
    {
    protected:
        virtual void dsRead(uint8_t *dst, int sz) = 0;

    private:
        template<typename T> T _dsReadT()
        {
            uint8_t rd[sizeof(T)];

            if(m_ed == Endianness::Little)
                dsRead(rd, sizeof(T));
            else {
                uint8_t tmp[sizeof(T)];
                dsRead(tmp, sizeof(T));

                for(uint8_t i = 0; i < sizeof(T); i++)
                    rd[i] = tmp[sizeof(T) - i - 1];
            }

            return *reinterpret_cast<T*>(rd);
        }

    public:
        //Constructor
        DataDeserializer()
        {
            m_ed = Endianness::Little;
            m_stringMode = StringSerialization::UShortLenAndContent;
        }

        DataDeserializer(Endianness ed)
        {
            m_ed = ed;
            m_stringMode = StringSerialization::UShortLenAndContent;
        }

        virtual ~DataDeserializer()
        {
        }

        //Read functions
        char readChar()
        {
            char ret;
            dsRead(reinterpret_cast<uint8_t*>(&ret), 1);

            return ret;
        }

        int16_t readShort()
        {
            int16_t ret;
            dsRead(reinterpret_cast<uint8_t*>(&ret), 2);

            if(m_ed == Endianness::Little)
                return ret;
            else
                return ((ret & 0x00FF) << 8) | ((ret & 0xFF00) >> 8);
        }

        int32_t readInt()
        {
            int32_t ret;
            dsRead(reinterpret_cast<uint8_t*>(&ret), 4);

            if(m_ed == Endianness::Little)
                return ret;
            else
                return ((ret & 0x000000FF) << 24) | ((ret & 0x0000FF00) << 8) | ((ret & 0x00FF0000) >> 8) | ((ret & 0xFF000000) >> 24);
        }

        int64_t readInt64()
        {
            return _dsReadT<int64_t>();
        }

        uint8_t readByte()
        {
            uint8_t ret;
            dsRead(&ret, 1);

            return ret;
        }

        uint16_t readUShort()
        {
            int16_t ret = readShort();
            return *reinterpret_cast<uint16_t*>(&ret);
        }

        uint32_t readUInt()
        {
            int32_t ret = readInt();
            return *reinterpret_cast<uint32_t*>(&ret);
        }

        uint64_t readUInt64()
        {
            return _dsReadT<uint64_t>();
        }

        bool readBool()
        {
            uint8_t data;
            dsRead(&data, 1);

            return data != 0;
        }

        float readFloat()
        {
            static_assert(sizeof(float) == 4, "invalid float size");

            int32_t ret = readInt();
            return *reinterpret_cast<float*>(&ret);
        }

        double readDouble()
        {
            return _dsReadT<double>();
        }

        String readString()
        {
            switch(m_stringMode) {
            case StringSerialization::AppendNullByte:
            {
                String ret;
                char c;
                dsRead(reinterpret_cast<uint8_t*>(&c), 1);

                while(c != 0) {
                    ret += c;
                    dsRead(reinterpret_cast<uint8_t*>(&c), 1);
                }

                return ret;
            }

            case StringSerialization::ByteLenAndContent:
            {
                uint8_t sz;
                dsRead(&sz, 1);

                String ret(String::uninitialized(static_cast<int>(sz)));
                dsRead(reinterpret_cast<uint8_t*>(ret.begin()), static_cast<int>(sz));
                ret.begin()[sz] = 0;

                return ret;
            }

            case StringSerialization::UShortLenAndContent:
            {
                uint16_t sz;
                *this >> sz;

                String ret(String::uninitialized(static_cast<int>(sz)));
                dsRead(reinterpret_cast<uint8_t*>(ret.begin()), static_cast<int>(sz));
                ret.begin()[sz] = 0;

                return ret;
            }

            default:
                //Should never happen
                return String();
            }
        }

        //Read operators
        DataDeserializer &operator >> (char &c)
        {
            dsRead(reinterpret_cast<uint8_t*>(&c), 1);
            return *this;
        }

        DataDeserializer &operator >> (int16_t &data)
        {
            if(m_ed == Endianness::Little)
                dsRead(reinterpret_cast<uint8_t*>(&data), 2);
            else {
                int16_t tmp;
                dsRead(reinterpret_cast<uint8_t*>(&tmp), 2);

                data = ((tmp & 0x00FF) << 8) | ((tmp & 0xFF00) >> 8);
            }

            return *this;
        }

        DataDeserializer &operator >> (int32_t &data)
        {
            if(m_ed == Endianness::Little)
                dsRead(reinterpret_cast<uint8_t*>(&data), 4);
            else {
                int32_t tmp;
                dsRead(reinterpret_cast<uint8_t*>(&tmp), 4);

                data = ((tmp & 0x000000FF) << 24) | ((tmp & 0x0000FF00) << 8) | ((tmp & 0x00FF0000) >> 8) | ((tmp & 0xFF000000) >> 24);
            }

            return *this;
        }

        DataDeserializer &operator >> (int64_t &data)
        {
            data = _dsReadT<int64_t>();
            return *this;
        }

        DataDeserializer &operator >> (uint8_t &c)
        {
            dsRead(&c, 1);
            return *this;
        }

        DataDeserializer &operator >> (uint16_t &data)
        {
            return *this >> *reinterpret_cast<int16_t*>(&data);
        }

        DataDeserializer &operator >> (uint32_t &data)
        {
            return *this >> *reinterpret_cast<int32_t*>(&data);
        }

        DataDeserializer &operator >> (uint64_t &data)
        {
            data = _dsReadT<uint64_t>();
            return *this;
        }

        DataDeserializer &operator >> (String &data)
        {
            data = readString();
            return *this;
        }

        DataDeserializer &operator >> (bool &data)
        {
            uint8_t tmp;
            dsRead(&tmp, 1);

            data = tmp != 0;
            return *this;
        }

        DataDeserializer &operator >> (float &data)
        {
            static_assert(sizeof(float) == 4, "invalid float size");

            int32_t ret = readInt();
            data = *reinterpret_cast<float*>(&ret);
            return *this;
        }

        DataDeserializer &operator >> (double &data)
        {
            data = readDouble();
            return *this;
        }

        //Getters & setters
        Endianness endianness() const
        {
            return m_ed;
        }

        void setEndianness(Endianness ed)
        {
            m_ed = ed;
        }

        StringSerialization stringMode() const
        {
            return m_stringMode;
        }

        void setStringMode(StringSerialization stringMode)
        {
            m_stringMode = stringMode;
        }

    protected:
        Endianness m_ed;
        StringSerialization m_stringMode;
    };

}

