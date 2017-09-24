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
#include "String.h"
#include "IOStream.h"

#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#endif

//TODO: Non-copyable, move override

namespace m
{
    class MGPCL_PREFIX FileInputStream : public InputStream
    {
    public:
        enum OpenError
        {
            kOE_Success = 0,
            kOE_FileNotFound,
            kOE_Unknown
        };

        FileInputStream();
        FileInputStream(const String &fname);
        ~FileInputStream() override;

        OpenError open(const String &fname);
        int read(uint8_t *dst, int sz) override;
        uint64_t pos() override;
        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override;
        void close() override;

        bool seekSupported() const override
        {
            return true;
        }

    private:
#ifdef MGPCL_WIN
        HANDLE m_file;
#else
        int m_file;
#endif
    };

    class MGPCL_PREFIX FileOutputStream : public OutputStream
    {
    public:
        enum OpenMode
        {
            kOM_Normal = 0,
            kOM_Truncate,
            kOM_AtEnd
        };

        FileOutputStream();
        FileOutputStream(const String &fname, OpenMode mode);
        ~FileOutputStream() override;

        bool open(const String &fname, OpenMode mode);
        int write(const uint8_t *src, int sz) override;
        uint64_t pos() override;
        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override;
        bool flush() override;
        void close() override;

        bool seekSupported() const override
        {
            return true;
        }

    private:
#ifdef MGPCL_WIN
        HANDLE m_file;
#else
        int m_file;
#endif
    };
}
