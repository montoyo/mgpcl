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

#include "mgpcl/BasicLogger.h"
#include "mgpcl/Thread.h"

void m::BasicLogger::vlog(LogLevel level, const char *fname, int line, const char *format, VAList *lst)
{
    int fnameLen = 0;
    int slashPos = 0;

    while(fname[fnameLen] != 0) {
        if(fname[fnameLen] == '/' || fname[fnameLen] == '\\')
            slashPos = fnameLen + 1;

        fnameLen++;
    }

    String str(32);
    str += '[';

    switch(level) {
    case LogLevel::Debug:
        str += 'd';
        break;

    case LogLevel::Info:
        str += 'i';
        break;

    case LogLevel::Warning:
        str += 'W';
        break;

    case LogLevel::Error:
        str += '!';
        break;

    default:
        str += ' ';
        break;
    }

    str.append("] [", 3);

    String tName(Thread::currentThreadName());
    if(tName.length() > 6) {
        str.append(tName.raw(), 4);
        str.append("..] [", 5);
    } else {
        str += tName;
        for(int i = tName.length(); i < 6; i++)
            str += ' ';

        str.append("] [", 3);
    }

    str.append(fname + slashPos, fnameLen - slashPos);
    str += ':';
    str += String::fromUInteger(static_cast<uint32_t>(line));
    str.append("] ", 2);
    str += String::vformat(format, lst);
    
#ifdef MGPCL_WIN
    str.append("\r\n", 2);
#else
    str += '\n';
#endif

    m_lock.lock();

    if(level == LogLevel::Error && m_useErrStream)
        m_err.write(reinterpret_cast<const uint8_t*>(str.raw()), str.length());
    else
        m_out.write(reinterpret_cast<const uint8_t*>(str.raw()), str.length());

    m_lock.unlock();
}
