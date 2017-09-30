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
#include "Enums.h"
#include "String.h"
#include "SharedPtr.h"

namespace m
{
    template<class RefCnt> class TLineReader
    {
    public:
        TLineReader()
        {
            m_le = M_OS_LINEENDING;
            m_alloc = 8192;
            m_pos = 0;
            m_len = 0;
            m_buffer = new char[8192];
        }

        TLineReader(SharedPtr<InputStream, RefCnt> src) : m_source(src)
        {
            m_le = M_OS_LINEENDING;
            m_alloc = 8192;
            m_pos = 0;
            m_len = 0;
            m_buffer = new char[8192];
        }

        TLineReader(SharedPtr<InputStream, RefCnt> src, LineEnding le) : m_source(src)
        {
            m_le = le;
            m_alloc = 8192;
            m_pos = 0;
            m_len = 0;
            m_buffer = new char[8192];
        }

        TLineReader(TLineReader<RefCnt> &&src) : m_source(std::move(src.m_source)), m_line(std::move(src.m_line))
        {
            m_le = src.m_le;
            m_alloc = src.m_alloc;
            m_pos = src.m_pos;
            m_buffer = src.m_buffer;
            m_len = src.m_len;

            src.m_buffer = nullptr;
        }

        ~TLineReader()
        {
            if(m_buffer != nullptr)
                delete[] m_buffer;
        }

        void setLineEnding(LineEnding le)
        {
            m_le = le;
        }

        void setMaxLineLength(uint32_t limit)
        {
            mDebugAssert(limit >= 2, "buffer is too small");

            if(limit != m_alloc) {
                delete[] m_buffer;

                m_buffer = new char[limit];
                m_alloc = limit;
            }
        }

        void setSource(SharedPtr<InputStream, RefCnt> src)
        {
            m_source = src;
            m_pos = 0;
            m_len = 0;
        }

        LineEnding lineEnding() const
        {
            return m_le;
        }

        uint32_t maxLineLength() const
        {
            return m_alloc;
        }

        SharedPtr<InputStream, RefCnt> source() const
        {
            return m_source;
        }

        //Reads the next line. Use line() to access result. Returns -1 on error, 0 if stream ended, otherwise, if everything went fine, 1
        int next()
        {
            const char match = (m_le == LineEnding::LFOnly) ? '\n' : '\r';
            bool found = false;

            do {
                //Start over from pos, new line delimiter should NOT be located before
                while(m_pos < m_len && !found) {
                    if(m_buffer[m_pos] == match) {
                        if(m_le == LineEnding::CRLF) {
                            if(m_pos + 1 < m_len) {
                                if(m_buffer[m_pos + 1] == '\n')
                                    found = true;
                                else
                                    m_pos++;
                            } else
                                break; //Break to refill the buffer
                        } else
                            found = true;
                    } else
                        m_pos++;
                }

                if(!found) {
                    int ret = refillBuffer();

                    if(ret <= 0)
                        return ret;
                }
            } while(!found);

            //So here we have our line, m_buffer[m_pos] == 'line ending'
            const uint32_t leLen = (m_le == LineEnding::CRLF) ? 2 : 1;

            m_line.cleanup();
            m_line.append(m_buffer, m_pos);

            //Bring back
            m_len -= m_pos + leLen;
            if(m_len > 0)
                mem::move(m_buffer, m_buffer + m_pos + leLen, m_len);

            //Reset
            m_pos = 0;
            return 1;
        }

        const String &line() const
        {
            return m_line;
        }

        const char *remainingData() const
        {
            return m_buffer;
        }

        uint32_t remainingDataLength() const
        {
            return m_len;
        }

    private:
        int refillBuffer()
        {
            mDebugAssert(!m_source.isNull(), "forgot to set source");
            if(m_len >= m_alloc) //Make sure there's space in the buffer
                return -1;

            int ret = m_source->read(reinterpret_cast<uint8_t*>(m_buffer + m_len), m_alloc - m_len);
            if(ret <= 0)
                return ret;

            m_len += ret;
            return 1;
        }

        SharedPtr<InputStream, RefCnt> m_source;
        LineEnding m_le;
        uint32_t m_pos;
        uint32_t m_len;
        uint32_t m_alloc;
        char *m_buffer;
        String m_line;
    };

    typedef TLineReader<RefCounter> LineReader;
    typedef TLineReader<AtomicRefCounter> MTLineReader;

}
