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
#include "Enums.h"
#include <cstdint>

namespace m
{
    class MGPCL_PREFIX InputStream
    {
    public:
        virtual ~InputStream() {}
        virtual int read(uint8_t *dst, int sz) = 0;
        virtual uint64_t pos() = 0;
        virtual bool seek(int amount, SeekPos sp = SeekPos::Beginning) = 0;
        virtual bool seekSupported() const = 0;

        virtual void close() = 0;
    };

    class MGPCL_PREFIX OutputStream
    {
    public:
        virtual ~OutputStream() {}
        virtual int write(const uint8_t *src, int sz) = 0;
        virtual uint64_t pos() = 0;
        virtual bool seek(int amount, SeekPos sp = SeekPos::Beginning) = 0;
        virtual bool seekSupported() const = 0;
        virtual bool flush() = 0;

        virtual void close() = 0;
    };

    class MGPCL_PREFIX IOException
    {
    public:
        IOException(RWAction action) throw()
        {
            m_action = action;
        }

        const char *what() const throw()
        {
            return m_action == RWAction::Reading ? "failed to read data" : "failed to write data";
        }

    private:
        IOException()
        {    
        }

        RWAction m_action;
    };
}
