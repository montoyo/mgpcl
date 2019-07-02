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
#include "Assert.h"

#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <cstdio>
#include <unistd.h>
#endif

namespace m
{
    class STDOutputStream : public OutputStream
    {
    public:
        STDOutputStream()
        {
#ifdef MGPCL_WIN
            m_handle = INVALID_HANDLE_VALUE;
#else
            m_fd = -1;
#endif
        }

        STDOutputStream(STDHandle h)
        {
            setHandle(h);
        }

        ~STDOutputStream() override
        {
        }

        bool setHandle(STDHandle h)
        {
#ifdef MGPCL_WIN
            switch(h) {
            case STDHandle::HInput:
                m_handle = GetStdHandle(STD_INPUT_HANDLE);
                break;

            case STDHandle::HOutput:
                m_handle = GetStdHandle(STD_OUTPUT_HANDLE);
                break;

            case STDHandle::HError:
                m_handle = GetStdHandle(STD_ERROR_HANDLE);
                break;

            default:
                m_handle = INVALID_HANDLE_VALUE;
                break;
            }

            return m_handle != INVALID_HANDLE_VALUE;
#else
            switch(h) {
            case STDHandle::HInput:
                m_fd = STDIN_FILENO;
                break;

            case STDHandle::HOutput:
                m_fd = STDOUT_FILENO;
                break;

            case STDHandle::HError:
                m_fd = STDERR_FILENO;
                break;

            default:
                m_fd = -1;
                break;
            }

            return m_fd >= 0;
#endif
        }

        int write(const uint8_t *src, int sz) override
        {
            mDebugAssert(sz >= 0, "cannot write a negative amount of bytes");

#ifdef MGPCL_WIN
            mDebugAssert(m_handle != INVALID_HANDLE_VALUE, "stdoutputstream is invalid");

            DWORD ret;
            return WriteFile(m_handle, src, static_cast<DWORD>(sz), &ret, nullptr) == FALSE ? -1 : static_cast<int>(ret);
#else
            mDebugAssert(m_fd < 0, "stdoutputstream is invalid");
            return static_cast<int>(::write(m_fd, src, static_cast<size_t>(sz)));
#endif
        }

        uint64_t pos() override
        {
            return 0;
        }

        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
        {
            return false;
        }

        bool seekSupported() const override
        {
            return false;
        }

        bool flush() override
        {
#ifdef MGPCL_WIN
            mDebugAssert(m_handle != INVALID_HANDLE_VALUE, "stdoutputstream is invalid");
            return FlushFileBuffers(m_handle) != FALSE;
#else
            mDebugAssert(m_fd < 0, "stdoutputstream is invalid");
            return fsync(m_fd) == 0;
#endif
        }

        void close() override
        {
        }

    private:
#ifdef MGPCL_WIN
        HANDLE m_handle;
#else
        int m_fd;
#endif
    };

    class STDInputStream : public InputStream
    {
    public:
        STDInputStream()
        {
#ifdef MGPCL_WIN
            m_handle = INVALID_HANDLE_VALUE;
#else
            m_fd = -1;
#endif
        }

        STDInputStream(STDHandle h)
        {
            setHandle(h);
        }

        ~STDInputStream() override
        {
        }

        bool setHandle(STDHandle h)
        {
#ifdef MGPCL_WIN
            switch(h) {
            case STDHandle::HInput:
                m_handle = GetStdHandle(STD_INPUT_HANDLE);
                break;

            case STDHandle::HOutput:
                m_handle = GetStdHandle(STD_OUTPUT_HANDLE);
                break;

            case STDHandle::HError:
                m_handle = GetStdHandle(STD_ERROR_HANDLE);
                break;

            default:
                m_handle = INVALID_HANDLE_VALUE;
                break;
            }

            return m_handle != INVALID_HANDLE_VALUE;
#else
            switch(h) {
            case STDHandle::HInput:
                m_fd = STDIN_FILENO;
                break;

            case STDHandle::HOutput:
                m_fd = STDOUT_FILENO;
                break;

            case STDHandle::HError:
                m_fd = STDERR_FILENO;
                break;

            default:
                m_fd = -1;
                break;
            }

            return m_fd >= 0;
#endif
        }

        int read(uint8_t *src, int sz) override
        {
            mDebugAssert(sz >= 0, "cannot read a negative amount of bytes");

#ifdef MGPCL_WIN
            mDebugAssert(m_handle != INVALID_HANDLE_VALUE, "stdoutputstream is invalid");

            DWORD ret;
            return ReadFile(m_handle, src, static_cast<DWORD>(sz), &ret, nullptr) == FALSE ? -1 : static_cast<int>(ret);
#else
            mDebugAssert(m_fd < 0, "stdoutputstream is invalid");
            return static_cast<int>(::write(m_fd, src, static_cast<size_t>(sz)));
#endif
        }

        uint64_t pos() override
        {
            return 0;
        }

        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
        {
            return false;
        }

        bool seekSupported() const override
        {
            return false;
        }

        void close() override
        {
        }

    private:
#ifdef MGPCL_WIN
        HANDLE m_handle;
#else
        int m_fd;
#endif
    };

}
