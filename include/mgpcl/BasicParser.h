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
#include "Util.h"

namespace m
{

    enum BasicParserState
    {
        kBPS_Parsing = 0,
        kBPS_ReachedEOF,
        kBPS_CouldntRead
    };
    
    template<class RefCnt> class TBasicParser
    {
        M_NON_COPYABLE_T(TBasicParser, RefCnt)

    public:
        TBasicParser()
        {
            //Line & char counter
            m_numLine = 1;
            m_numChar = 0;

            //Buffer
            m_buf = new char[8192];
            m_bufLen = 0;
            m_bufPos = 0;

            //State
            m_state = kBPS_Parsing;
            m_last = 0;
        }

        TBasicParser(SharedPtr<InputStream, RefCnt> src) : m_src(src)
        {
            //Line & char counter
            m_numLine = 1;
            m_numChar = 0;

            //Buffer
            m_buf = new char[8192];
            m_bufLen = 0;
            m_bufPos = 0;

            //State
            m_state = kBPS_Parsing;
            m_last = 0;
        }

        ~TBasicParser()
        {
            if(m_buf != nullptr)
                delete[] m_buf;
        }

        bool refill()
        {
            int rd = m_src->read(reinterpret_cast<uint8_t*>(m_buf), 8192);
            if(rd == 0) {
                m_state = kBPS_ReachedEOF;
                return false;
            } else if(rd < 0) {
                m_state = kBPS_CouldntRead;
                return false;
            }

            m_bufLen = static_cast<uint32_t>(rd);
            m_bufPos = 0;
            return true;
        }

        int nextCharRaw() //Returns -1 on EOF/error
        {
            if(m_bufPos >= m_bufLen && !refill())
                return -1;

            char ret = m_buf[m_bufPos++];
            if(ret == '\r') {
                m_numLine++;
                m_numChar = 0;
            } else if(ret == '\n') {
                if(m_last != '\r') {
                    m_numLine++;
                    m_numChar = 0;
                }
            } else
                m_numChar++;

            m_last = ret;
            return static_cast<int>(ret);
        }

        int nextNonBlankChar() //Returns -1 on EOF/error
        {
            int ret;
            do {
                ret = nextCharRaw();
            } while(ret >= 0 && (ret == '\r' || ret == '\n' || ret == '\t' || ret == ' '));

            return ret;
        }

        bool expect(const char *str, int len = -1, bool caseSensitive = true)
        {
            if(len < 0)
                len = static_cast<int>(strlen(str));

            if(caseSensitive) {
                for(int i = 0; i < len; i++) {
                    if(nextCharRaw() != str[i])
                        return false;
                }
            } else {
                for(int i = 0; i < len; i++) {
                    if(tolower(nextCharRaw()) != tolower(str[i]))
                        return false;
                }
            }

            return true;
        }

        void undo()
        {
            mAssert(m_bufPos > 0, "can't undo twice");
            m_bufPos--;
        }

        int line() const
        {
            return m_numLine;
        }

        int column() const
        {
            return m_numChar;
        }

        BasicParserState state() const
        {
            return m_state;
        }

        SharedPtr<InputStream, RefCnt> source() const
        {
            return m_src;
        }

        void setSource(SharedPtr<InputStream, RefCnt> src)
        {
            m_src = src;
            m_bufLen = 0; //Invalidate internal buffer
            m_numLine = 1; //Reset counters
            m_numChar = 0;
            m_state = kBPS_Parsing; //Reset state
        }

    private:
        SharedPtr<InputStream, RefCnt> m_src;

        char *m_buf;
        uint32_t m_bufLen;
        uint32_t m_bufPos;

        int m_numLine;
        int m_numChar;

        char m_last;
        BasicParserState m_state;
    };

    typedef TBasicParser<RefCounter> BasicParser;
    typedef TBasicParser<AtomicRefCounter> MTBasicParser;

}
