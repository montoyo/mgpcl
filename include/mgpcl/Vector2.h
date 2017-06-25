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
    template<typename T> class Vector2
    {
    public:
        Vector2()
        {
            m_x = T(0);
            m_y = T(0);
        }

        Vector2(T x, T y)
        {
            m_x = x;
            m_y = y;
        }

        Vector2(T val)
        {
            m_x = val;
            m_y = val;
        }

        T x() const
        {
            return m_x;
        }

        T y() const
        {
            return m_y;
        }

        void setX(T val)
        {
            m_x = val;
        }

        void setY(T val)
        {
            m_y = val;
        }

        void set(T x, T y)
        {
            m_x = x;
            m_y = y;
        }

        void set(T val)
        {
            m_x = val;
            m_y = val;
        }

        bool isNull() const
        {
            return m_x == T(0) && m_y == T(0);
        }

        bool operator == (const Vector2<T> &src)
        {
            return m_x == src.m_x && m_y == src.m_y;
        }

        bool operator != (const Vector2<T> &src)
        {
            return m_x != src.m_x || m_y != src.m_y;
        }

        template<typename Other> Vector2<Other> cast() const
        {
            return Vector2<Other>(static_cast<Other>(m_x), static_cast<Other>(m_y));
        }

        T length2() const
        {
            return m_x * m_x + m_y * m_y;
        }

        T length() const
        {
            return Math::sqrt<T>(m_x * m_x + m_y * m_y);
        }

        T dist2(const Vector2<T> &src) const
        {
            T dx = src.m_x - m_x;
            T dy = src.m_y - m_y;

            return dx * dx + dy * dy;
        }

        T dist(const Vector2<T> &src) const
        {
            T dx = src.m_x - m_x;
            T dy = src.m_y - m_y;

            return Math::sqrt<T>(dx * dx + dy * dy);
        }

        Vector2<T> &normalize()
        {
            T len = Math::sqrt<T>(m_x * m_x + m_y * m_y);
            m_x /= len;
            m_y /= len;

            return *this;
        }

        Vector2<T> normalized() const
        {
            T len = Math::sqrt<T>(m_x * m_x + m_y * m_y);
            return Vector2<T>(m_x / len, m_y / len);
        }

        Vector2<T> &operator += (const Vector2<T> &src)
        {
            m_x += src.m_x;
            m_y += src.m_y;

            return *this;
        }

        Vector2<T> &operator -= (const Vector2<T> &src)
        {
            m_x -= src.m_x;
            m_y -= src.m_y;

            return *this;
        }

        Vector2<T> &operator *= (const Vector2<T> &src)
        {
            m_x *= src.m_x;
            m_y *= src.m_y;

            return *this;
        }

        Vector2<T> &operator /= (const Vector2<T> &src)
        {
            m_x /= src.m_x;
            m_y /= src.m_y;

            return *this;
        }

        Vector2<T> &operator += (T val)
        {
            m_x += val;
            m_y += val;

            return *this;
        }

        Vector2<T> &operator -= (T val)
        {
            m_x -= val;
            m_y -= val;

            return *this;
        }

        Vector2<T> &operator *= (T val)
        {
            m_x *= val;
            m_y *= val;

            return *this;
        }

        Vector2<T> &operator /= (T val)
        {
            m_x /= val;
            m_y /= val;

            return *this;
        }

        Vector2<T> operator + (const Vector2<T> &src) const
        {
            return Vector2<T>(m_x + src.m_x, m_y + src.m_y);
        }

        Vector2<T> operator - (const Vector2<T> &src) const
        {
            return Vector2<T>(m_x - src.m_x, m_y - src.m_y);
        }

        Vector2<T> operator * (const Vector2<T> &src) const
        {
            return Vector2<T>(m_x * src.m_x, m_y * src.m_y);
        }

        Vector2<T> operator / (const Vector2<T> &src) const
        {
            return Vector2<T>(m_x / src.m_x, m_y / src.m_y);
        }

        Vector2<T> operator + (T val) const
        {
            return Vector2<T>(m_x + val, m_y + val);
        }

        Vector2<T> operator - (T val) const
        {
            return Vector2<T>(m_x - val, m_y - val);
        }

        Vector2<T> operator * (T val) const
        {
            return Vector2<T>(m_x * val, m_y * val);
        }

        Vector2<T> operator / (T val) const
        {
            return Vector2<T>(m_x / val, m_y / val);
        }

        Vector2<T> operator - () const
        {
            return Vector2<T>(-m_x, -m_y);
        }

    private:
        T m_x;
        T m_y;
    };

    typedef Vector2<float> Vector2f;
    typedef Vector2<double> Vector2d;
    typedef Vector2<int> Vector2i;
}

