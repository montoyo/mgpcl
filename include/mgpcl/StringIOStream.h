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
#include "IOStream.h"
#include "String.h"

namespace m
{
    class StringOStream : public OutputStream
    {
    public:
        StringOStream()
        {
            m_pos = 0;
            m_fill = ' ';
            m_insert = false;
        }

        ~StringOStream() override
        {
        }

        int write(const uint8_t *dst, int sz) override;
        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override;

        uint64_t pos() override
        {
            return static_cast<uint64_t>(m_pos);
        }

        bool seekSupported() const override
        {
            return true;
        }

        bool flush() override
        {
            return true;
        }

        void close() override
        {
        }

        const String &data() const
        {
            return m_data;
        }

        void clear()
        {
            m_data.clear();
        }
        
        void cleanup()
        {
            m_data.cleanup();
        }

        bool isEmpty() const
        {
            return m_data.isEmpty();
        }

        void setInserts(bool i)
        {
            m_insert = i;
        }

        void setReplaces(bool r)
        {
            m_insert = !r;
        }

        void setFillCharacter(char f)
        {
            m_fill = f;
        }

        bool inserts() const
        {
            return m_insert;
        }

        bool replaces() const
        {
            return !m_insert;
        }

        char fillCharacter() const
        {
            return m_fill;
        }

    private:
        String m_data;
        int m_pos;
        bool m_insert;
        char m_fill;
    };

    class StringIStream : public InputStream
    {
    public:
        StringIStream()
        {
            m_pos = 0;
        }

        StringIStream(const String &src) : m_data(src)
        {
            m_pos = 0;
        }

        ~StringIStream() override
        {
        }

        int read(uint8_t *dst, int sz) override;
        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override;

        uint64_t pos() override
        {
            return static_cast<uint64_t>(m_pos);
        }

        bool seekSupported() const override
        {
            return true;
        }

        void close() override
        {
        }

        const String &data() const
        {
            return m_data;
        }

    private:
        String m_data;
        int m_pos;
    };

}
