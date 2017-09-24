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
#include "Vector3.h"

namespace m
{
    template <typename T>
    class Ray
    {
    public:
        Ray()
        {
        }

        Ray(const Vector3<T> &s, const Vector3<T> &p) : m_s(s), m_p(p)
        {
        }

        Ray(T sx, T sy, T sz, T px, T py, T pz) : m_s(sx, sy, sz), m_p(px, py, pz)
        {
        }

        Ray(const Vector3<T> &p) : m_p(p)
        {
        }

        Ray(T px, T py, T pz) : m_p(px, py, pz)
        {
        }

        Vector3<T> at(T t) const
        {
            return m_s + m_p * t;
        }

        Vector3<T> operator[](T t) const
        {
            return m_s + m_p * t;
        }

        Ray<T> &advance(T t)
        {
            m_s += m_p * t;
            return *this;
        }

        Ray<T> operator +(T t) const
        {
            return Ray<T>(m_s + m_p * t, m_p);
        }

        Ray<T> &operator +=(T t)
        {
            m_s += m_p * t;
            return *this;
        }

        Ray<T> operator -(T t) const
        {
            return Ray<T>(m_s - m_p * t, m_p);
        }

        Ray<T> &operator -=(T t)
        {
            m_s -= m_p * t;
            return *this;
        }

        Ray<T> operator +(const Vector3<T> &v) const
        {
            return Ray<T>(m_s + v, m_p);
        }

        Ray<T> &operator +=(const Vector3<T> &v)
        {
            m_s += v;
            return *this;
        }

        Ray<T> operator -(const Vector3<T> &v) const
        {
            return Ray<T>(m_s - v, m_p);
        }

        Ray<T> &operator -=(const Vector3<T> &v)
        {
            m_s -= v;
            return *this;
        }

        Ray<T> operator -() const
        {
            return Ray<T>(m_s, -m_p);
        }

        Vector3<T> &s()
        {
            return m_s;
        }

        Vector3<T> &p()
        {
            return m_p;
        }

        const Vector3<T> &s() const
        {
            return m_s;
        }

        const Vector3<T> &p() const
        {
            return m_p;
        }

        Ray<T> &set(const Vector3<T> &s, const Vector3<T> &t)
        {
            m_s = s;
            m_p = p;
            return *this;
        }

        Ray<T> &set(T sx, T sy, T sz, T px, T py, T pz)
        {
            m_s.set(sx, sy, sz);
            m_p.set(px, py, pz);
            return *this;
        }

        Ray<T> &setS(const Vector3<T> &s)
        {
            m_s = s;
            return *this;
        }

        Ray<T> &setS(T x, T y, T z)
        {
            m_s.set(x, y, z);
            return *this;
        }

        Ray<T> &setP(const Vector3<T> &p)
        {
            m_p = p;
            return *this;
        }

        Ray<T> &setP(T x, T y, T z)
        {
            m_p.set(x, y, z);
            return *this;
        }

        T length() const
        {
            return m_p.length();
        }

        T length2() const
        {
            return m_p.length2();
        }

        Ray<T> &normalize()
        {
            m_p.normalize();
            return *this;
        }

        Ray<T> normalized() const
        {
            return Ray<T>(m_s, m_p.normalized());
        }

        bool isNull() const
        {
            return m_p.length2() == T(0.0);
        }

    private:
        Vector3<T> m_s;
        Vector3<T> m_p;
    };

    typedef Ray<float> Rayf;
    typedef Ray<double> Rayd;
}
