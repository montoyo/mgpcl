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
#include "MathConstants.h"

//This might be useful if I want to write
//my own math lib one day...

namespace m
{
    namespace Math
    {
        template<typename T> T sin(T ang)
        {
            return std::sin(ang);
        }

        template<typename T> T cos(T ang)
        {
            return std::cos(ang);
        }

        template<typename T> T tan(T ang)
        {
            return std::tan(ang);
        }

        template<typename T> T sqrt(T x)
        {
            return std::sqrt(x);
        }

        template<typename T> T asin(T x)
        {
            return std::asin(x);
        }

        template<typename T> T acos(T x)
        {
            return std::acos(x);
        }

        template<typename T> T atan(T x)
        {
            return std::atan(x);
        }

        template<typename T> T atan2(T y, T x)
        {
            return std::atan2(y, x);
        }

        template<typename T> T pow(T x, T p)
        {
            return std::pow(x, p);
        }

        template<typename T> T maximum(T a, T b)
        {
            return a > b ? a : b;
        }

        template<typename T> T minimum(T a, T b)
        {
            return a < b ? a : b;
        }

        template<typename T> T clamp(T x, T xmin, T xmax)
        {
            if(x < xmin)
                return xmin;
            else if(x > xmax)
                return xmax;
            else
                return x;
        }
    }
}

