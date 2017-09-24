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
#include "Assert.h"
#include "Util.h"
#include "Mem.h"

namespace m
{
    template<class RefCnt> class MGPCL_PREFIX TBufferedOutputStream : public OutputStream
    {
    public:
        TBufferedOutputStream()
        {
            m_buf = new uint8_t[8192];
            m_alloc = 8192;
            m_pos = 0;
        }

        TBufferedOutputStream(uint32_t bufSz)
        {
            m_buf = new uint8_t[bufSz];
            m_alloc = bufSz;
            m_pos = 0;
        }

        TBufferedOutputStream(const SharedPtr<InputStream, RefCnt> &c, uint32_t bufSz = 8192)
        {
            m_child = c;
            m_buf = new uint8_t[bufSz];
            m_alloc = bufSz;
            m_pos = 0;
        }

        ~TBufferedOutputStream() override
        {
            if(!m_child.isNull())
                _flush();

            delete[] m_buf;
        }

        int write(const uint8_t *src, int sz) override
        {
            mDebugAssert(!m_child.isNull(), "child not set");
            mDebugAssert(sz > 0, "cannot write negative amount of bytes");

            int ret = sz;
            while(sz > 0) {
                uint32_t toWrite = minimum(static_cast<uint32_t>(sz), m_alloc - m_pos);
                Mem::copy(m_buf + m_pos, src, toWrite);
                sz -= static_cast<int>(toWrite);
                src += toWrite;
                m_pos += toWrite;

                if(m_pos >= m_alloc && !_flush())
                    return -1;
            }

            return ret;
        }

        uint64_t pos() override
        {
            mDebugAssert(!m_child.isNull(), "child not set");
            return m_child->pos() + static_cast<uint64_t>(m_pos);
        }

        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
        {
            mDebugAssert(!m_child.isNull(), "child not set");
            return _flush() && m_child->seek(amount, sp);
        }

        bool seekSupported() const override
        {
            return true;
        }

        bool flush() override
        {
            mDebugAssert(!m_child.isNull(), "child not set");
            return _flush() && m_child->flush();
        }

        void close() override
        {
            mDebugAssert(!m_child.isNull(), "child not set");

            _flush();
            m_child->close();
        }

        const SharedPtr<OutputStream, RefCnt> &child() const
        {
            return m_child;
        }

        void setChild(const SharedPtr<OutputStream, RefCnt> &outputStream)
        {
            m_child = outputStream;
        }

    private:
        bool _flush()
        {
            uint8_t *ptr = m_buf;
            while(m_pos > 0) {
                int written = m_child->write(ptr, static_cast<int>(m_pos));
                if(written <= 0)
                    return false;

                ptr += written;
                m_pos -= static_cast<uint32_t>(written);
            }

            return true;
        }

        SharedPtr<OutputStream, RefCnt> m_child;
        uint8_t *m_buf;
        uint32_t m_pos;
        uint32_t m_alloc;
    };

    typedef TBufferedOutputStream<RefCounter> BufferedOutputStream;
    typedef TBufferedOutputStream<AtomicRefCounter> MTBufferedOutputStream;

}
