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
#include "SharedPtr.h"
#include "Enums.h"
#include "String.h"

namespace m
{
    template<class RefCnt> class TLineOutputStream : public OutputStream
    {
    public:
        TLineOutputStream()
        {
            m_hadCR = false;
            m_le = M_OS_LINEENDING;
        }

        TLineOutputStream(const SharedPtr<OutputStream, RefCnt> &child) : m_child(child)
        {
            m_hadCR = false;
            m_le = M_OS_LINEENDING;
        }

        TLineOutputStream(LineEnding le)
        {
            m_hadCR = false;
            m_le = le;
        }

        TLineOutputStream(LineEnding le, const SharedPtr<OutputStream, RefCnt> &child) : m_child(child)
        {
            m_hadCR = false;
            m_le = le;
        }

        ~TLineOutputStream() override
        {
        }

        int write(const uint8_t *src_, int sz) override
        {
            //All the magic happens here
            const char *src = reinterpret_cast<const char*>(src_);
            int last = 0;
            int err;

            char newLine[2];
            int nlSize;
            switch(m_le) {
            case LineEnding::CRLF:
                newLine[0] = '\r';
                newLine[1] = '\n';
                nlSize = 2;
                break;

            case LineEnding::LFOnly:
                newLine[0] = '\n';
                nlSize = 1;
                break;

            case LineEnding::CROnly:
                newLine[0] = '\r';
                nlSize = 1;
                break;

            default:
                nlSize = 0;
                break;
            }

            for(int i = 0; i < sz; i++) {
                if(src[i] == '\r' || src[i] == '\n') {
                    if(m_hadCR) {
                        //Special treatment for CRLF
                        m_hadCR = false;

                        if(src[i] == '\n')
                            continue;
                    }

                    //Write previous data
                    if(i > last) {
                        err = writeAll(src + last, i - last);

                        if(err <= 0)
                            return err;
                    }

                    //Write new line
                    err = writeAll(newLine, nlSize);
                    if(err <= 0)
                        return err;

                    if(src[i] == '\r') {
                        if(i + 1 >= sz)
                            m_hadCR = true;
                        else if(src[i + 1] == '\n')
                            i++; //Skip dat
                    }

                    last = i + 1;
                } else if(m_hadCR)
                    m_hadCR = false;
            }

            if(last < sz) {
                //We still have some data to write!
                err = writeAll(src + last, sz - last);
                if(err <= 0)
                    return err;
            }

            return sz;
        }

        bool write(const String &str)
        {
            return write(reinterpret_cast<const uint8_t*>(str.raw()), str.length()) == str.length();
        }

        bool put(char c)
        {
            return write(reinterpret_cast<const uint8_t*>(&c), 1) == 1;
        }

        bool writeLine(const char *str, int sz)
        {
            if(sz < 0) {
                sz = 0;
                while(str[sz] != 0)
                    sz++;
            }

            if(write(reinterpret_cast<const uint8_t*>(str), sz) != sz)
                return false;

            bool status;
            switch(m_le) {
            case LineEnding::CRLF:
                status = (writeAll("\r\n", 2) > 0);
                break;

            case LineEnding::LFOnly:
                status = (writeAll("\n", 1) > 0);
                break;

            case LineEnding::CROnly:
                status = (writeAll("\r", 1) > 0);
                break;

            default:
                status = true;
                break;
            }

            return status;
        }

        bool writeLine(const String &str)
        {
            return writeLine(str.raw(), str.length());
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
            m_child->close();
        }

        const SharedPtr<OutputStream, RefCnt> &child() const
        {
            return m_child;
        }

        void setChild(const SharedPtr<OutputStream, RefCnt> &child)
        {
            m_child = child;
        }

        LineEnding lineEnding() const
        {
            return m_le;
        }

        void setLineEnding(LineEnding le)
        {
            m_le = le;
        }

    private:
        bool m_hadCR;
        LineEnding m_le;
        SharedPtr<OutputStream, RefCnt> m_child;

        int writeAll(const char *str, int len)
        {
            int ret = len;

            while(len > 0) {
                int rd = m_child->write(reinterpret_cast<const uint8_t*>(str), len);
                if(rd <= 0)
                    return rd;

                str += rd;
                len -= rd;
            }

            return ret;
        }
    };

    typedef TLineOutputStream<RefCounter> LineOutputStream;
    typedef TLineOutputStream<AtomicRefCounter> MTLineOutputStream;
}
