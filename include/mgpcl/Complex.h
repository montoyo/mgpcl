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
#include "Math.h"

namespace m
{
    template<typename T> class Complex
    {
    public:
        Complex()
        {
            m_a = T(0);
            m_b = T(0);
        }

        Complex(T val)
        {
            m_a = val;
            m_b = val;
        }

        Complex(T a, T b)
        {
            m_a = a;
            m_b = b;
        }

        T real() const
        {
            return m_a;
        }

        T imag() const
        {
            return m_b;
        }

        void setReal(T val)
        {
            m_a = val;
        }

        void setImag(T val)
        {
            m_b = val;
        }

        void set(T a, T b)
        {
            m_a = a;
            m_b = b;
        }

        void set(T val)
        {
            m_a = val;
            m_b = val;
        }

        bool operator == (const Complex<T> &src) const
        {
            return m_a == src.m_a && m_b == src.m_b;
        }

        bool operator != (const Complex<T> &src) const
        {
            return m_a != src.m_a || m_b != src.m_b;
        }

        Complex<T> &operator += (const Complex<T> &src)
        {
            m_a += src.m_a;
            m_b += src.m_b;

            return *this;
        }

        Complex<T> &operator += (T val)
        {
            m_a += val;
            return *this;
        }

        Complex<T> &operator -= (const Complex<T> &src)
        {
            m_a -= src.m_a;
            m_b -= src.m_b;

            return *this;
        }

        Complex<T> &operator -= (T val)
        {
            m_a -= val;
            return *this;
        }

        Complex<T> &operator *= (const Complex<T> &src)
        {
            T a = m_a * src.m_a - m_b * src.m_b;
            m_b = m_b * src.m_a + m_a * src.m_b;
            m_a = a;
            
            return *this;
        }
        
        Complex<T> &operator *= (T val)
        {
            m_a *= val;
            m_b *= val;
            
            return *this;
        }

        Complex<T> &operator /= (const Complex<T> &src)
        {
            T div = src.m_a * src.m_a + src.m_b * src.m_b;
            T   a = (m_a * src.m_a + m_b * src.m_b) / div;
            m_b   = (m_b * src.m_a - m_a * src.m_b) / div;
            m_a   = a;

            return *this;
        }

        Complex<T> &operator /= (T val)
        {
            m_a /= val;
            m_b /= val;

            return *this;
        }

        Complex<T> operator + (const Complex &src) const
        {
            return Complex<T>(m_a + src.m_a, m_b + src.m_b);
        }

        Complex<T> operator + (T val) const
        {
            return Complex<T>(m_a + val, m_b);
        }

        Complex<T> operator - (const Complex &src) const
        {
            return Complex<T>(m_a - src.m_a, m_b - src.m_b);
        }

        Complex<T> operator - (T val) const
        {
            return Complex<T>(m_a - val, m_b);
        }

        Complex<T> operator - () const
        {
            return Complex<T>(-m_a, -m_b);
        }

        Complex<T> operator * (const Complex<T> &src) const
        {
            return Complex<T>(m_a * src.m_a - m_b * src.m_b, m_b * src.m_a + m_a * src.m_b);
        }

        Complex<T> operator * (T val) const
        {
            return Complex<T>(m_a * val, m_b * val);
        }

        Complex<T> operator / (const Complex<T> &src) const
        {
            T div = src.m_a * src.m_b + src.m_b * src.m_b;
            return Complex<T>((m_a * src.m_a + m_b * src.m_b) / div, (m_b * src.m_a - m_a * src.m_b) / div);
        }

        Complex<T> operator / (T val) const
        {
            return Complex<T>(m_a / val, m_b / val);
        }

        Complex<T> conj() const
        {
            return Complex<T>(m_a, -m_b);
        }

        T abs2() const
        {
            return m_a * m_a + m_b * m_b;
        }

        T abs() const
        {
            return sqrt(m_a * m_a + m_b * m_b);
        }

        T arg() const
        {
            return atan2(m_b, m_a);
        }

        static Complex<T> fromPolar(T len, T arg)
        {
            return Complex<T>(len * Math::cos(arg), len * Math::sin(arg));
        }

    private:
        T m_a;
        T m_b;
	};

}

