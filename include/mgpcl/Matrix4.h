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
#include "Vector3.h"
#include "Quaternion.h"
#include "Mem.h"
#include "Math.h"

#define M_SET_ROW(row, a, b, c, d) m_data[0][row] = T(a); m_data[1][row] = T(b); m_data[2][row] = T(c); m_data[3][row] = T(d);
#define M_SET_ROW_OF(mat, row, a, b, c, d) (mat)->m_data[0][row] = T(a); (mat)->m_data[1][row] = T(b); (mat)->m_data[2][row] = T(c); (mat)->m_data[3][row] = T(d);

namespace m
{
    template<typename T> class Matrix4
    {
    public:
        Matrix4<T> &loadIdentity()
        {
            M_SET_ROW(0,   1, 0, 0, 0);
            M_SET_ROW(1,   0, 1, 0, 0);
            M_SET_ROW(2,   0, 0, 1, 0);
            M_SET_ROW(3,   0, 0, 0, 1);

            return *this;
        }

        Matrix4<T> &translate(T x, T y, T z = T(0))
        {
            Matrix4<T> trans;
            trans.loadIdentity();
            trans.m_data[3][0] = x;
            trans.m_data[3][1] = y;
            trans.m_data[3][2] = z;

            (*this) *= trans;
            return *this;
        }

        Matrix4<T> &translate(const Vector3<T> &v)
        {
            Matrix4<T> trans;
            trans.loadIdentity();
            trans.m_data[3][0] = v.x();
            trans.m_data[3][1] = v.y();
            trans.m_data[3][2] = v.z();

            (*this) *= trans;
            return *this;
        }

        Matrix4<T> &translate(const Vector2<T> &v)
        {
            Matrix4<T> trans;
            trans.loadIdentity();
            trans.m_data[3][0] = v.x();
            trans.m_data[3][1] = v.y();

            (*this) *= trans;
            return *this;
        }

        Matrix4<T> &scale(T x, T y, T z = T(1))
        {
            Matrix4<T> trans;
            mem::zero(trans.m_data, sizeof(T) * 4 * 4);
            trans.m_data[0][0] = x;
            trans.m_data[1][1] = y;
            trans.m_data[2][2] = z;
            trans.m_data[3][3] = T(1);

            (*this) *= trans;
            return *this;
        }

        Matrix4<T> &scale(T val)
        {
            Matrix4<T> trans;
            mem::zero(trans.m_data, sizeof(T) * 4 * 4);
            trans.m_data[0][0] = val;
            trans.m_data[1][1] = val;
            trans.m_data[2][2] = val;
            trans.m_data[3][3] = T(1);

            (*this) *= trans;
            return *this;
        }

        Matrix4<T> &scale(const Vector3<T> &v)
        {
            Matrix4<T> trans;
            mem::zero(trans.m_data, sizeof(T) * 4 * 4);
            trans.m_data[0][0] = v.x();
            trans.m_data[1][1] = v.y();
            trans.m_data[2][2] = v.z();
            trans.m_data[3][3] = T(1);

            (*this) *= trans;
            return *this;
        }

        Matrix4<T> &scale(const Vector2<T> &v)
        {
            Matrix4<T> trans;
            mem::zero(trans.m_data, sizeof(T) * 4 * 4);
            trans.m_data[0][0] = v.x();
            trans.m_data[1][1] = v.y();
            trans.m_data[2][2] = T(1);
            trans.m_data[3][3] = T(1);

            (*this) *= trans;
            return *this;
        }

        Matrix4<T> &rotateX(T theta)
        {
            Matrix4<T> trans;
            trans.loadIdentity();

            T sint = math::sin(theta);
            T cost = math::cos(theta);

            trans.m_data[1][1] = cost;
            trans.m_data[2][1] = -sint;
            trans.m_data[1][2] = sint;
            trans.m_data[2][2] = cost;

            (*this) *= trans;
            return *this;
        }

        Matrix4<T> &rotateY(T theta)
        {
            Matrix4<T> trans;
            trans.loadIdentity();

            T sint = math::sin(theta);
            T cost = math::cos(theta);

            trans.m_data[0][0] = cost;
            trans.m_data[2][0] = sint;
            trans.m_data[0][2] = -sint;
            trans.m_data[2][2] = cost;

            (*this) *= trans;
            return *this;
        }

        Matrix4<T> &rotateZ(T theta)
        {
            Matrix4<T> trans;
            trans.loadIdentity();

            T sint = math::sin(theta);
            T cost = math::cos(theta);

            trans.m_data[0][0] = cost;
            trans.m_data[1][0] = -sint;
            trans.m_data[0][1] = sint;
            trans.m_data[1][1] = cost;

            (*this) *= trans;
            return *this;
        }
        
        Matrix4<T> &rotate(T x, T y, T z, T w)
        {
            Matrix4<T> trans;
            trans.loadIdentity();

            trans.m_data[0][0] = T(1) - T(2) * (y * y + z * z);
            trans.m_data[0][1] = T(2) * (x * y + z * w);
            trans.m_data[0][2] = T(2) * (x * z - y * w);

            trans.m_data[1][0] = T(2) * (x * y - z * w);
            trans.m_data[1][1] = T(1) - T(2) * (x * x + z * z);
            trans.m_data[1][2] = T(2) * (y * z + x * w);

            trans.m_data[2][0] = T(2) * (x * z + y * w);
            trans.m_data[2][1] = T(2) * (y * z - x * w);
            trans.m_data[2][2] = T(1) - T(2) * (x * x + y * y);

            (*this) *= trans;
            return *this;
        }
        
        Matrix4<T> &rotate(const Quaternion<T> &q)
        {
            Matrix4<T> trans;
            trans.loadIdentity();

            trans.m_data[0][0] = T(1) - T(2) * (q.y() * q.y() + q.z() * q.z());
            trans.m_data[0][1] = T(2) * (q.x() * q.y() + q.z() * q.w());
            trans.m_data[0][2] = T(2) * (q.x() * q.z() - q.y() * q.w());

            trans.m_data[1][0] = T(2) * (q.x() * q.y() - q.z() * q.w());
            trans.m_data[1][1] = T(1) - T(2) * (q.x() * q.x() + q.z() * q.z());
            trans.m_data[1][2] = T(2) * (q.y() * q.z() + q.x() * q.w());

            trans.m_data[2][0] = T(2) * (q.x() * q.z() + q.y() * q.w());
            trans.m_data[2][1] = T(2) * (q.y() * q.z() - q.x() * q.w());
            trans.m_data[2][2] = T(1) - T(2) * (q.x() * q.x() + q.y() * q.y());

            (*this) *= trans;
            return *this;
        }

        Matrix4<T> &lookAt(const Vector3<T> &eye, const Vector3<T> &center, const Vector3<T> &up)
        {
            Vector3<T> view((center - eye).normalize());
            Vector3<T> normal(view.cross(up).normalize());
            Vector3<T> axis(normal.cross(view).normalize());

            Matrix4<T> trans;
            trans.loadIdentity();

            trans.m_data[0][0] = normal.x();
            trans.m_data[1][0] = normal.y();
            trans.m_data[2][0] = normal.z();

            trans.m_data[0][1] = axis.x();
            trans.m_data[1][1] = axis.y();
            trans.m_data[2][1] = axis.z();

            trans.m_data[0][2] = -view.x();
            trans.m_data[1][2] = -view.y();
            trans.m_data[2][2] = -view.z();
            (*this) *= trans;

            //Also translate
            trans.loadIdentity();
            trans.m_data[3][0] = -eye.x();
            trans.m_data[3][1] = -eye.y();
            trans.m_data[3][2] = -eye.z();

            (*this) *= trans;
            return *this;
        }

        Matrix4<T> &lookAt(T eyeX, T eyeY, T eyeZ, T centerX, T centerY, T centerZ, T upX, T upY, T upZ)
        {
            return lookAt(Vector3<T>(eyeX, eyeY, eyeZ), Vector3<T>(centerX, centerY, centerZ), Vector3<T>(upX, upY, upZ));
        }

        T *data()
        {
            return reinterpret_cast<T*>(m_data);
        }

        const T *data() const
        {
            return reinterpret_cast<const T*>(m_data);
        }

        T value(int col, int row) const
        {
            return m_data[col][row];
        }

        void setValue(int col, int row, T val)
        {
            m_data[col][row] = val;
        }

        Matrix4<T> inverted() const
        {
            //WARNING: Probably very slow!!
            Matrix4<T> ret;
            ret.loadIdentity();

            Matrix4<T> me(*this);

            for(int x = 0; x < 4; x++) {
                int y = x;
                while(y < 4 && me.m_data[x][y] == T(0))
                    y++;

                if(x != y) {
                    for(int i = 0; i < 4; i++) {
                        swap(me.m_data[i][x], me.m_data[i][y]);
                        swap(ret.m_data[i][x], ret.m_data[i][y]);
                    }
                }

                for(y = 0; y < 4; y++) {
                    if(x == y) {
                        T f = me.m_data[x][y];

                        for(int i = 0; i < 4; i++) {
                            me.m_data[i][y] /= f;
                            ret.m_data[i][y] /= f;
                        }
                    } else if(me.m_data[x][y] != 0) {
                        T f = -me.m_data[x][y] / me.m_data[x][x];

                        for(int i = 0; i < 4; i++) {
                            me.m_data[i][y] += f * me.m_data[i][x];
                            ret.m_data[i][y] += f * ret.m_data[i][x];
                        }
                    }
                }
            }

            return ret;
        }

        Vector3<T> multiply(const Vector3<T> &src, T win, T &wout) const
        {
            Vector3<T> ret;
            ret.setX(m_data[0][0] * src.x() + m_data[1][0] * src.y() + m_data[2][0] * src.z() + m_data[3][0] * win);
            ret.setY(m_data[0][1] * src.x() + m_data[1][1] * src.y() + m_data[2][1] * src.z() + m_data[3][1] * win);
            ret.setZ(m_data[0][2] * src.x() + m_data[1][2] * src.y() + m_data[2][2] * src.z() + m_data[3][2] * win);
            wout  =  m_data[0][3] * src.x() + m_data[1][3] * src.y() + m_data[2][3] * src.z() + m_data[3][3] * win;

            return ret;
        }
        
        Vector3<T> multiply(const Vector3<T> &src, T win) const
        {
            Vector3<T> ret;
            ret.setX(m_data[0][0] * src.x() + m_data[1][0] * src.y() + m_data[2][0] * src.z() + m_data[3][0] * win);
            ret.setY(m_data[0][1] * src.x() + m_data[1][1] * src.y() + m_data[2][1] * src.z() + m_data[3][1] * win);
            ret.setZ(m_data[0][2] * src.x() + m_data[1][2] * src.y() + m_data[2][2] * src.z() + m_data[3][2] * win);

            return ret;
        }
        
        Vector3<T> multiplyEx(const Vector3<T> &src, T &wout) const
        {
            Vector3<T> ret;
            ret.setX(m_data[0][0] * src.x() + m_data[1][0] * src.y() + m_data[2][0] * src.z());
            ret.setY(m_data[0][1] * src.x() + m_data[1][1] * src.y() + m_data[2][1] * src.z());
            ret.setZ(m_data[0][2] * src.x() + m_data[1][2] * src.y() + m_data[2][2] * src.z());
            wout  =  m_data[0][3] * src.x() + m_data[1][3] * src.y() + m_data[2][3] * src.z();

            return ret;
        }

        Matrix4<T> &operator *= (const Matrix4<T> &src)
        {
            T tmp[4][4];

            for(int x = 0; x < 4; x++) {
                tmp[x][0] = m_data[0][0] * src.m_data[x][0] + m_data[1][0] * src.m_data[x][1] + m_data[2][0] * src.m_data[x][2] + m_data[3][0] * src.m_data[x][3];
                tmp[x][1] = m_data[0][1] * src.m_data[x][0] + m_data[1][1] * src.m_data[x][1] + m_data[2][1] * src.m_data[x][2] + m_data[3][1] * src.m_data[x][3];
                tmp[x][2] = m_data[0][2] * src.m_data[x][0] + m_data[1][2] * src.m_data[x][1] + m_data[2][2] * src.m_data[x][2] + m_data[3][2] * src.m_data[x][3];
                tmp[x][3] = m_data[0][3] * src.m_data[x][0] + m_data[1][3] * src.m_data[x][1] + m_data[2][3] * src.m_data[x][2] + m_data[3][3] * src.m_data[x][3];
            }

            mem::copy(m_data, tmp, 4 * 4 * sizeof(float));
            return *this;
        }

        Matrix4<T> operator * (const Matrix4<T> &src) const
        {
            Matrix4<T> ret;

            for(int x = 0; x < 4; x++) {
                ret.m_data[x][0] = m_data[0][0] * src.m_data[x][0] + m_data[1][0] * src.m_data[x][1] + m_data[2][0] * src.m_data[x][2] + m_data[3][0] * src.m_data[x][3];
                ret.m_data[x][1] = m_data[0][1] * src.m_data[x][0] + m_data[1][1] * src.m_data[x][1] + m_data[2][1] * src.m_data[x][2] + m_data[3][1] * src.m_data[x][3];
                ret.m_data[x][2] = m_data[0][2] * src.m_data[x][0] + m_data[1][2] * src.m_data[x][1] + m_data[2][2] * src.m_data[x][2] + m_data[3][2] * src.m_data[x][3];
                ret.m_data[x][3] = m_data[0][3] * src.m_data[x][0] + m_data[1][3] * src.m_data[x][1] + m_data[2][3] * src.m_data[x][2] + m_data[3][3] * src.m_data[x][3];
            }

            return ret;
        }

        Vector3<T> operator * (const Vector3<T> &src) const
        {
            Vector3<T> ret;
            ret.setX(m_data[0][0] * src.x() + m_data[1][0] * src.y() + m_data[2][0] * src.z() + m_data[3][0]);
            ret.setY(m_data[0][1] * src.x() + m_data[1][1] * src.y() + m_data[2][1] * src.z() + m_data[3][1]);
            ret.setZ(m_data[0][2] * src.x() + m_data[1][2] * src.y() + m_data[2][2] * src.z() + m_data[3][2]);

            return ret;
        }

        Matrix4<T> transposed() const
        {
            Matrix4<T> ret;
            M_SET_ROW_OF(&ret, 0, m_data[0][0], m_data[0][1], m_data[0][2], m_data[0][3]);
            M_SET_ROW_OF(&ret, 1, m_data[1][0], m_data[1][1], m_data[1][2], m_data[1][3]);
            M_SET_ROW_OF(&ret, 2, m_data[2][0], m_data[2][1], m_data[2][2], m_data[2][3]);
            M_SET_ROW_OF(&ret, 3, m_data[3][0], m_data[3][1], m_data[3][2], m_data[3][3]);

            return ret;
        }

        Matrix4<T> operator ~ () const
        {
            return transposed();
        }

        Matrix4<T> &transpose()
        {
            *this = transposed();
            return *this;
        }

        static Matrix4<T> perspective(T angle, T ratio, T near_, T far_)
        {
            Matrix4<T> ret;

            float f = T(1) / math::tan(angle * T(0.5));
            M_SET_ROW_OF(&ret, 0,   f / ratio, T(0), T(0),  T(0));
            M_SET_ROW_OF(&ret, 1,   T(0),      f,    T(0),  T(0));
            M_SET_ROW_OF(&ret, 2,   T(0),      T(0), (near_ + far_) / (near_ - far_), (T(2) * near_ * far_) / (near_ - far_));
            M_SET_ROW_OF(&ret, 3,   T(0),      T(0), T(-1), T(0));

            return ret;
        }

        static Matrix4<T> ortho(T left, T bottom, T right, T top)
        {
            Matrix4<T> ret;
            ret.loadIdentity();

            ret.m_data[0][0] = T(2) / (right - left);
            ret.m_data[1][1] = T(2) / (top - bottom);
            ret.m_data[2][2] = T(-1);
            ret.m_data[3][0] = -((right + left) / (right - left));
            ret.m_data[3][1] = -((top + bottom) / (top - bottom));

            return ret;
        }

        static Matrix4<T> ortho(T left, T bottom, T right, T top, T near_, T far_)
        {
            Matrix4<T> ret;
            ret.loadIdentity();

            ret.m_data[0][0] = T(2) / (right - left);
            ret.m_data[1][1] = T(2) / (top - bottom);
            ret.m_data[2][2] = T(-2) / (far_ - near_);
            ret.m_data[3][0] = -((right + left) / (right - left));
            ret.m_data[3][1] = -((top + bottom) / (top - bottom));
            ret.m_data[3][2] = -((far_ + near_) / (far_ - near_));

            return ret;
        }
        
    private:
        static void swap(T &a, T &b)
        {
            T tmp = a;
            a = b;
            b = tmp;
        }

        T m_data[4][4];
    };

    typedef Matrix4<float> Matrix4f;
    typedef Matrix4<double> Matrix4d;
    typedef Matrix4<int> Matrix4i;
}

#undef M_SET_ROW
#undef M_SET_ROW_OF

