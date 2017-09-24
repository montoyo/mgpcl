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
#include "IOStream.h"
#include "SharedPtr.h"
#include "Mem.h"
#include "Assert.h"
#include "List.h"

namespace m
{
    class MGPCL_PREFIX BasicBufferStream
    {
    public:
        bool _seek(int amount, SeekPos sp)
        {
            switch(sp) {
            case SeekPos::Beginning:
                mDebugAssert(amount >= 0, "cannot seek backward from beginning");
                if(static_cast<uint32_t>(amount) > m_len)
                    return false;

                m_ptr = static_cast<uint32_t>(amount);
                break;

            case SeekPos::Relative:
                if(amount > 0) {
                    if(m_ptr + static_cast<uint32_t>(amount) > m_len)
                        return false;

                    m_ptr += static_cast<uint32_t>(amount);
                } else {
                    amount = -amount;
                    if(static_cast<uint32_t>(amount) > m_ptr)
                        return false;

                    m_ptr -= static_cast<uint32_t>(amount);
                }
                break;

            case SeekPos::End:
                mDebugAssert(amount < 0, "cannot seek forward from the end");
                amount = -amount;

                if(static_cast<uint32_t>(amount) > m_len)
                    return false;

                m_ptr = m_len - static_cast<uint32_t>(amount);
                break;

            default:
                return false;
            }

            return true;
        }

    protected:
        uint32_t m_len;
        uint32_t m_ptr;
    };

    class MGPCL_PREFIX BufferInputStream : public InputStream, protected BasicBufferStream
    {
    public:
        BufferInputStream()
        {
            m_data = nullptr;
            m_len = 0;
            m_ptr = 0;
        }

        BufferInputStream(const uint8_t *data, uint32_t len)
        {
            m_data = data;
            m_len = len;
            m_ptr = 0;
        }

        ~BufferInputStream() override
        {
        }

        void setData(const uint8_t *data, uint32_t len)
        {
            m_data = data;
            m_len = len;
            m_ptr = 0;
        }

        const uint8_t *data() const
        {
            return m_data;
        }

        uint32_t length() const
        {
            return m_len;
        }

        int read(uint8_t *dst, int sz) override
        {
            mDebugAssert(m_data != nullptr, "called BufferIOStream::read() with NULL data");
            mDebugAssert(sz >= 0, "cannot read a negative amount of bytes");

            if(m_len - m_ptr == 0)
                sz = 0;
            else if(sz > 0) {
                if(static_cast<uint32_t>(sz) > m_len - m_ptr)
                    sz = static_cast<uint32_t>(m_len - m_ptr);

                Mem::copy(dst, m_data + m_ptr, static_cast<size_t>(sz));
                m_ptr += static_cast<uint32_t>(sz);
            }

            return sz;
        }

        uint64_t pos() override
        {
            return static_cast<uint64_t>(m_ptr);
        }

        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
        {
            return _seek(amount, sp);
        }

        bool seekSupported() const override
        {
            return true;
        }

        void close() override
        {
            m_data = nullptr;
        }

    private:
        const uint8_t *m_data;
    };

    class MGPCL_PREFIX BufferOutputStream : public OutputStream, protected BasicBufferStream
    {
    public:
        BufferOutputStream()
        {
            m_data = nullptr;
            m_len = 0;
            m_ptr = 0;
        }

        BufferOutputStream(uint8_t *data, uint32_t len)
        {
            m_data = data;
            m_len = len;
            m_ptr = 0;
        }

        ~BufferOutputStream() override
        {
        }

        void setData(uint8_t *data, uint32_t len)
        {
            m_data = data;
            m_len = len;
            m_ptr = 0;
        }

        uint8_t *data()
        {
            return m_data;
        }

        uint32_t length() const
        {
            return m_len;
        }

        int write(const uint8_t *src, int sz) override
        {
            mDebugAssert(m_data != nullptr, "called BufferIOStream::write() with NULL data");
            mDebugAssert(sz >= 0, "cannot write a negative amount of bytes");

            if(m_len - m_ptr == 0)
                sz = 0;
            else if(sz > 0) {
                if(static_cast<uint32_t>(sz) > m_len - m_ptr)
                    sz = static_cast<uint32_t>(m_len - m_ptr);

                Mem::copy(m_data + m_ptr, src, static_cast<size_t>(sz));
                m_ptr += static_cast<uint32_t>(sz);
            }

            return sz;
        }

        bool flush() override
        {
            return true;
        }

        uint64_t pos() override
        {
            return static_cast<uint64_t>(m_ptr);
        }

        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
        {
            return _seek(amount, sp);
        }

        bool seekSupported() const override
        {
            return true;
        }

        void close() override
        {
            m_data = nullptr;
        }

    private:
        uint8_t *m_data;
    };

}

