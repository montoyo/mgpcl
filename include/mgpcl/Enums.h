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

namespace m
{

    enum class SeekPos
    {
        Beginning,
        Relative,
        End
    };

    enum class RWAction
    {
        Reading,
        Writing
    };

    enum class Endianness
    {
        Little,
        Big
    };

    enum class StringSerialization
    {
        AppendNullByte,
        ByteLenAndContent,
        UShortLenAndContent
    };

    enum class LineEnding
    {
        CRLF,   //Windows
        LFOnly, //Linux
        CROnly  //Mac
    };

    enum class STDHandle
    {
        HInput,
        HOutput,
        HError
    };

#ifdef MGPCL_WIN
#define M_OS_LINEENDING ::m::LineEnding::CRLF
#define M_OS_LINEEND "\r\n"
#else
#define M_OS_LINEENDING ::m::LineEnding::LFOnly
#define M_OS_LINEEND "\n"
#endif

    enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Error
    };

    enum class Axis
    {
        X,
        Y,
        Z
    };

    enum class MouseButton
    {
        None,
        Left,
        Middle,
        Right
    };

}
