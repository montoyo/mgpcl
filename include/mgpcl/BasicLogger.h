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
#include "Logger.h"
#include "Mutex.h"
#include "STDIOStream.h"
#include "Util.h"

namespace m
{

    class BasicLogger : public Logger
    {
        M_NON_COPYABLE(BasicLogger)

    public:
        BasicLogger() : m_out(STDHandle::HOutput), m_err(STDHandle::HError)
        {
            m_useErrStream = true;
            m_fnamePad = 24;
        }

        ~BasicLogger() override
        {
        }

        void vlog(LogLevel level, const char *fname, int line, const char *format, VAList *lst) override;

        bool usesErrorStream() const
        {
            return m_useErrStream;
        }

        /* This method is not thread safe.
         * Please use it at the very beginning of your program,
         * before the logger is used by another thread.
         */
        void setUsesErrorStream(bool err)
        {
            m_useErrStream = err;
        }

        void setFileNamePadding(int p)
        {
            m_fnamePad = p;
        }

        int fileNamePadding() const
        {
            return m_fnamePad;
        }

    private:
        Mutex m_lock;
        STDOutputStream m_out;
        STDOutputStream m_err;
        bool m_useErrStream;
        int m_fnamePad;
    };

}

