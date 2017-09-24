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
#include "TextSerializer.h"
#include "SharedPtr.h"

namespace m
{
    
    template<class RefCnt> class TTextOutputStream : public OutputStream, public TextSerializer
    {
    public:
        TTextOutputStream()
        {
        }

        TTextOutputStream(SharedPtr<OutputStream, RefCnt> child) : m_child(child)
        {
        }

        TTextOutputStream(SharedPtr<OutputStream, RefCnt> child, LineEnding le) : TextSerializer(le), m_child(child)
        {
        }

        int write(const uint8_t *src, int sz) override
        {
            return m_child->write(src, sz);
        }

        uint64_t pos() override
        {
            return m_child->pos();
        }

        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
        {
            return m_child->seek(amount, sp);
        }

        bool seekSupported() const override
        {
            return m_child->seekSupported();
        }

        bool flush() override
        {
            return m_child->flush();
        }

        void close() override
        {
            return m_child->close();
        }

        SharedPtr<OutputStream, RefCnt> child() const
        {
            return m_child;
        }

        void setChild(SharedPtr<OutputStream, RefCnt> child)
        {
            m_child = child;
        }

    protected:
        void tsWrite(const char *txt, int len) override
        {
            while(len > 0) {
                int ret = m_child->write(reinterpret_cast<const uint8_t*>(txt), len);
                if(ret <= 0)
                    return;

                txt += ret;
                len -= ret;
            }
        }

    private:
        SharedPtr<OutputStream, RefCnt> m_child;
    };

    template<class RefCnt> class TTextInputStream : public InputStream, public TextDeserializer
    {
    public:
        TTextInputStream()
        {
        }

        TTextInputStream(SharedPtr<InputStream, RefCnt> child) : m_child(child)
        {
        }

        int read(uint8_t *dst, int sz) override
        {
            return m_child->read(dst, sz);
        }

        uint64_t pos() override
        {
            return m_child->pos();
        }
        
        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
        {
            return m_child->seek(amount, sp);
        }
        
        bool seekSupported() const override
        {
            return m_child->seekSupported();
        }
        
        void close() override
        {
            m_child->close();
        }

        SharedPtr<InputStream, RefCnt> child() const
        {
            return m_child;
        }

        void setChild(SharedPtr<InputStream, RefCnt> child)
        {
            m_child = child;
        }

    protected:
        void tsRead(char *dst, int len) override
        {
            while(len > 0) {
                int rd = m_child->read(reinterpret_cast<uint8_t*>(dst), len);
                if(rd <= 0)
                    return;

                dst += rd;
                len -= rd;
            }
        }

    private:
        SharedPtr<InputStream, RefCnt> m_child;
    };

    typedef TTextOutputStream<RefCounter> TextOutputStream;
    typedef TTextOutputStream<AtomicRefCounter> MTTextOutputStream;
    typedef TTextInputStream<RefCounter> TextInputStream;
    typedef TTextInputStream<AtomicRefCounter> MTTextInputStream;

}
