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
#include "Math.h"

namespace m
{

    //Quaternion class only used to represent rotations in space
    //Here, we have q = xi + yj + zk + w (so w is the real part)
    template<typename T> class Quaternion
    {
    public:
        Quaternion()
        {
            m_x = T(0);
            m_y = T(0);
            m_z = T(0);
            m_w = T(0);
        }

        Quaternion(T x, T y, T z, T w)
        {
            m_x = x;
            m_y = y;
            m_z = z;
            m_w = w;
        }

        Quaternion(const Vector3<T> &im, T re = T(0))
        {
            m_x = im.x();
            m_y = im.y();
            m_z = im.z();
            m_w = re;
        }

        Quaternion(T val)
        {
            m_x = val;
            m_y = val;
            m_z = val;
            m_w = val;
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

        T w() const
        {
            return m_w;
        }

        Vector3<T> xyz() const
        {
            return Vector3<T>(m_x, m_y, m_z);
        }

        Quaternion<T> &setX(T val)
        {
            m_x = val;
            return *this;
        }

        Quaternion<T> &setY(T val)
        {
            m_y = val;
            return *this;
        }

        Quaternion<T> &setZ(T val)
        {
            m_z = val;
            return *this;
        }

        Quaternion<T> &setW(T val)
        {
            m_w = val;
            return *this;
        }

        Quaternion<T> &set(T x, T y, T z, T w)
        {
            m_x = x;
            m_y = y;
            m_z = z;
            m_w = w;
            return *this;
        }

        Quaternion<T> &set(const Vector3<T> &im, T re = T(0))
        {
            m_x = im.x();
            m_y = im.y();
            m_z = im.z();
            m_w = re;
            return *this;
        }

        Quaternion<T> &set(T val)
        {
            m_x = val;
            m_y = val;
            m_z = val;
            m_w = val;
            return *this;
        }

        //Rotations happen in the Y, X, Z order
        //From Wikipedia: "The airplane first does yaw (Y) during taxi,
        //then pitches (X) during take-off, and finally rolls (Z) in the air."
        Quaternion<T> &setFromEuler(T ax, T ay, T az)
        {
            T cosPhi = math::cos<T>(az / T(2)); //Roll
            T sinPhi = math::sin<T>(az / T(2));
            T cosTet = math::cos<T>(ax / T(2)); //Pitch (btw, it's supposed to be theta :D)
            T sinTet = math::sin<T>(ax / T(2));
            T cosPsi = math::cos<T>(ay / T(2)); //Yaw
            T sinPsi = math::sin<T>(ay / T(2));

            m_w = cosPhi * cosTet * cosPsi + sinPhi * sinTet * sinPsi;
            m_x = sinPhi * cosTet * cosPsi - cosPhi * sinTet * sinPsi;
            m_y = cosPhi * sinTet * cosPsi + sinPhi * cosTet * sinPsi;
            m_z = cosPhi * cosTet * sinPsi - sinPhi * sinTet * cosPsi;
            return *this;
        }

        Quaternion<T> &setFromEuler(const Vector3<T> &angles)
        {
            return setFromEuler(angles.x(), angles.y(), angles.z());
        }

        //Returns the pitch (theta)
        T eulerX() const
        {
            T a = T(2) * (m_w * m_x + m_y * m_z);
            T b = T(1) - T(2) * (m_x * m_x + m_y * m_y);

            return math::atan2<T>(a, b);
        }

        //Returns the yaw (psi)
        T eulerY() const
        {
            T a = T(2) * (m_w * m_z + m_x * m_y);
            T b = T(1) - T(2) * (m_z * m_z + m_y * m_y);

            return math::atan2<T>(a, b);
        }

        //Returns the roll (phi)
        T eulerZ() const
        {
            T val = math::clamp<T>(T(2) * (m_w * m_y - m_z * m_x), T(-1), T(1));
            return math::asin<T>(val);
        }

        Vector3<T> eulerAngles() const
        {
            T y2 = m_y * m_y;
            T xa = T(2) * (m_w * m_x + m_y * m_z);
            T xb = T(1) - T(2) * (m_x * m_x + y2);
            T ya = T(2) * (m_w * m_z + m_x * m_y);
            T yb = T(1) - T(2) * (m_z * m_z + y2);
            T z = math::clamp<T>(T(2) * (m_w * m_y - m_z * m_x), T(-1), T(1));

            return Vector3<T>(math::atan2<T>(xa, xb), math::atan2<T>(ya, yb), math::asin<T>(z));
        }

        Quaternion<T> operator + (const Quaternion<T> &src) const
        {
            return Quaternion<T>(m_x + src.m_x, m_y + src.m_y, m_z + src.m_z, m_w + src.m_w);
        }

        Quaternion<T> operator - (const Quaternion<T> &src) const
        {
            return Quaternion<T>(m_x - src.m_x, m_y - src.m_y, m_z - src.m_z, m_w - src.m_w);
        }

        Quaternion<T> operator - () const
        {
            return Quaternion<T>(-m_x, -m_y, -m_z, -m_w);
        }

        Quaternion<T> operator * (T t) const
        {
            return Quaternion<T>(m_x * t, m_y * t, m_z * t, m_w * t);
        }

        Quaternion<T> operator / (T t) const
        {
            return Quaternion<T>(m_x / t, m_y / t, m_z / t, m_w / t);
        }

        Quaternion<T> &operator += (const Quaternion<T> &src)
        {
            m_x += src.m_x;
            m_y += src.m_y;
            m_z += src.m_z;
            m_w += src.m_w;
            return *this;
        }

        Quaternion<T> &operator -= (const Quaternion<T> &src)
        {
            m_x -= src.m_x;
            m_y -= src.m_y;
            m_z -= src.m_z;
            m_w -= src.m_w;
            return *this;
        }

        Quaternion<T> &operator *= (T t)
        {
            m_x *= t;
            m_y *= t;
            m_z *= t;
            m_w *= t;
            return *this;
        }

        Quaternion<T> &operator /= (T t)
        {
            m_x /= t;
            m_y /= t;
            m_z /= t;
            m_w /= t;
            return *this;
        }

        Quaternion<T> &neg()
        {
            m_x = -m_x;
            m_y = -m_y;
            m_z = -m_z;
            m_w = -m_w;
            return *this;
        }

        Quaternion<T> interpolate(Quaternion<T> b, T t) const
        {
            T cosOmega = m_x * b.m_x + m_y * b.m_y + m_z * b.m_z + m_w * b.m_w;
            if(cosOmega < T(0)) {
                cosOmega = -cosOmega;
                b.neg();
            }

            T cp, cq;
            if(T(1) - cosOmega > T(0.0001)) {
                T omega = math::acos<T>(cosOmega);
                T sinOmega = math::sin<T>(omega);

                cp = math::sin<T>((T(1) - t) * omega) / sinOmega;
                cq = math::sin<T>(t * omega) / sinOmega;
            } else {
                //With small angles, lerp will work just fine...
                cp = T(1) - t;
                cq = t;
            }

            return *this * cp + b * cq;
        }

        template<typename Other> Quaternion<Other> cast() const
        {
            return Quaternion<Other>(static_cast<Other>(m_x), static_cast<Other>(m_y), static_cast<Other>(m_z), static_cast<Other>(m_w));
        }

    private:
        T m_x;
        T m_y;
        T m_z;
        T m_w;
    };

    typedef Quaternion<float> Quaternionf;
    typedef Quaternion<double> Quaterniond;

}

