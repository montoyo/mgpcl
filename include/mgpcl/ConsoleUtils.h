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
#include "String.h"
#include "Vector2.h"

#ifdef M_CUTILS_SRC
#define M_CUTILS_PREFIX
#else
#define M_CUTILS_PREFIX extern
#endif

namespace m
{
    enum ConsoleColor
    {
        kCC_Black = 0,
        kCC_Blue,
        kCC_Green,
        kCC_WeirdBlue,
        kCC_WeirdBrown,
        kCC_WeirdPurple,
        kCC_WeirdGreen,
        kCC_LightGray,
        kCC_Gray,
        kCC_LightBlue,
        kCC_LightGreen, //Matrix green
        kCC_Cyan,
        kCC_Red,
        kCC_Pink,
        kCC_Yellow,
        kCC_White
    };

    namespace console
    {
        M_CUTILS_PREFIX void setTitle(const String &title);
        M_CUTILS_PREFIX void setTextColor(ConsoleColor cc);
        M_CUTILS_PREFIX void setBackgroundColor(ConsoleColor cc);
        M_CUTILS_PREFIX void setColor(ConsoleColor fg, ConsoleColor bg);
        M_CUTILS_PREFIX Vector2i getSize();
        M_CUTILS_PREFIX Vector2i getCursorPos();
        M_CUTILS_PREFIX void setCursorPos(int x, int y);
        M_CUTILS_PREFIX void clearLastLine();
        M_CUTILS_PREFIX void clear();
        M_CUTILS_PREFIX void resetColor();

        inline void setCursorPos(const Vector2i &pos)
        {
            setCursorPos(pos.x(), pos.y());
        }
    }
}
