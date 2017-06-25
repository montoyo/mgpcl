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
#include "Vector2.h"
#include "Math.h"

namespace m
{
    template<typename T> class Vector3
    {
    public:
        Vector3()
        {
            m_x = T(0);
            m_y = T(0);
            m_z = T(0);
        }

        Vector3(T x, T y, T z)
        {
            m_x = x;
            m_y = y;
            m_z = z;
        }

        Vector3(T val)
        {
            m_x = val;
            m_y = val;
            m_z = val;
        }

        T x() const
        {
            return m_x;
        }

        T y() const
        {
            return m_y;
        }

        T z() const
        {
            return m_z;
        }

		Vector2<T> xy()
        {
			return Vector2<T>(m_x, m_y);
        }

        void setX(T val)
        {
            m_x = val;
        }

        void setY(T val)
        {
            m_y = val;
        }

        void setZ(T val)
        {
            m_z = val;
        }

        void set(T x, T y, T z)
        {
            m_x = x;
            m_y = y;
            m_z = z;
        }

        void set(T val)
        {
            m_x = val;
            m_y = val;
            m_z = val;
        }

        bool isNull() const
        {
            return m_x == T(0) && m_y == T(0) && m_z == T(0);
        }

        bool operator == (const Vector3<T> &src)
        {
            return m_x == src.m_x && m_y == src.m_y && m_z == src.m_z;
        }

        bool operator != (const Vector3<T> &src)
        {
            return m_x != src.m_x || m_y != src.m_y || m_z != src.m_z;
        }

        template<typename Other> Vector3<Other> cast() const
        {
            return Vector3<Other>(static_cast<Other>(m_x), static_cast<Other>(m_y), static_cast<Other>(m_z));
        }

        T dot(const Vector3<T> &src) const
        {
            return m_x * src.m_x + m_y * src.m_y + m_z * src.m_z;
        }

        T length2() const
        {
            return m_x * m_x + m_y * m_y + m_z * m_z;
        }

        T length() const
        {
            return Math::sqrt<T>(m_x * m_x + m_y * m_y + m_z * m_z);
        }

        T dist2(const Vector3<T> &src) const
        {
            T dx = src.m_x - m_x;
            T dy = src.m_y - m_y;
            T dz = src.m_z - m_z;

            return dx * dx + dy * dy + dz * dz;
        }

        T dist(const Vector3<T> &src) const
        {
            T dx = src.m_x - m_x;
            T dy = src.m_y - m_y;
            T dz = src.m_z - m_z;

            return Math::sqrt<T>(dx * dx + dy * dy + dz * dz);
        }

        Vector3<T> cross(const Vector3<T> &src) const
        {
            return Vector3<T>(  m_y * src.m_z - m_z * src.m_y,
                                m_z * src.m_x - m_x * src.m_z,
                                m_x * src.m_y - m_y * src.m_x);
        }

		//NOTE: the normal has to be normalized!
		Vector3<T> reflect(const Vector3<T> &normal) const
        {
			return *this - normal * (T(2.0) * dot(normal));
        }

		//NOTE: this has to be normalized!
		Vector2<T> unwrapSphere() const
        {
			T u = T(0.5) + Math::atan2<T>(m_z, m_x) / (T(2.0) * T(M_PI));
			T v = T(0.5) - Math::asin<T>(m_y) / T(M_PI);

			if(u < T(0.0))
				u = -u;

			if(v < T(0.0))
				v = -v;

			if(u >= T(1.0))
				u -= floor(u);

			if(v >= T(1.0))
				v -= floor(v);

			return Vector2<T>(u, v);
        }

        Vector3<T> &normalize()
        {
            T len = Math::sqrt<T>(m_x * m_x + m_y * m_y + m_z * m_z);
            m_x /= len;
            m_y /= len;
            m_z /= len;

            return *this;
        }

        Vector3<T> normalized() const
        {
            T len = Math::sqrt<T>(m_x * m_x + m_y * m_y + m_z * m_z);
            return Vector3<T>(m_x / len, m_y / len, m_z / len);
        }

        Vector3<T> &operator += (const Vector3<T> &src)
        {
            m_x += src.m_x;
            m_y += src.m_y;
            m_z += src.m_z;

            return *this;
        }

        Vector3<T> &operator -= (const Vector3<T> &src)
        {
            m_x -= src.m_x;
            m_y -= src.m_y;
            m_z -= src.m_z;

            return *this;
        }

        Vector3<T> &operator *= (const Vector3<T> &src)
        {
            m_x *= src.m_x;
            m_y *= src.m_y;
            m_z *= src.m_z;

            return *this;
        }

        Vector3<T> &operator /= (const Vector3<T> &src)
        {
            m_x /= src.m_x;
            m_y /= src.m_y;
            m_z /= src.m_z;

            return *this;
        }

        Vector3<T> &operator += (T val)
        {
            m_x += val;
            m_y += val;
            m_z += val;

            return *this;
        }

        Vector3<T> &operator -= (T val)
        {
            m_x -= val;
            m_y -= val;
            m_z -= val;

            return *this;
        }

        Vector3<T> &operator *= (T val)
        {
            m_x *= val;
            m_y *= val;
            m_z *= val;

            return *this;
        }

        Vector3<T> &operator /= (T val)
        {
            m_x /= val;
            m_y /= val;
            m_z /= val;

            return *this;
        }

        Vector3<T> operator + (const Vector3<T> &src) const
        {
            return Vector3<T>(m_x + src.m_x, m_y + src.m_y, m_z + src.m_z);
        }

        Vector3<T> operator - (const Vector3<T> &src) const
        {
            return Vector3<T>(m_x - src.m_x, m_y - src.m_y, m_z - src.m_z);
        }

        Vector3<T> operator * (const Vector3<T> &src) const
        {
            return Vector3<T>(m_x * src.m_x, m_y * src.m_y, m_z * src.m_z);
        }

        Vector3<T> operator / (const Vector3<T> &src) const
        {
            return Vector3<T>(m_x / src.m_x, m_y / src.m_y, m_z / src.m_z);
        }

        Vector3<T> operator + (T val) const
        {
            return Vector3<T>(m_x + val, m_y + val, m_z + val);
        }

        Vector3<T> operator - (T val) const
        {
            return Vector3<T>(m_x - val, m_y - val, m_z - val);
        }

        Vector3<T> operator * (T val) const
        {
            return Vector3<T>(m_x * val, m_y * val, m_z * val);
        }

        Vector3<T> operator / (T val) const
        {
            return Vector3<T>(m_x / val, m_y / val, m_z / val);
        }

        Vector3<T> operator - () const
        {
            return Vector3<T>(-m_x, -m_y, -m_z);
        }

    private:
        T m_x;
        T m_y;
        T m_z;
    };

    typedef Vector3<float> Vector3f;
    typedef Vector3<double> Vector3d;
    typedef Vector3<int> Vector3i;
}

