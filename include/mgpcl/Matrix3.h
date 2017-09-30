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
#include "Mem.h"
#include "Math.h"

#define M_SET_ROW(row, a, b, c) m_data[0][row] = T(a); m_data[1][row] = T(b); m_data[2][row] = T(c);
#define M_SET_ROW_OF(mat, row, a, b, c) (mat)->m_data[0][row] = T(a); (mat)->m_data[1][row] = T(b); (mat)->m_data[2][row] = T(c);

namespace m
{
    template<typename T> class Matrix3
    {
    public:
        Matrix3<T> &loadIdentity()
        {
            M_SET_ROW(0,   1, 0, 0);
            M_SET_ROW(1,   0, 1, 0);
            M_SET_ROW(2,   0, 0, 1);

            return *this;
        }

        Matrix3<T> &translate(T x, T y)
        {
            Matrix3<T> trans;
            trans.loadIdentity();
            trans.m_data[2][0] = x;
            trans.m_data[2][1] = y;

            (*this) *= trans;
            return *this;
        }

        Matrix3<T> &translate(const Vector2<T> &v)
        {
            Matrix3<T> trans;
            trans.loadIdentity();
            trans.m_data[2][0] = v.x();
            trans.m_data[2][1] = v.y();

            (*this) *= trans;
            return *this;
        }

        Matrix3<T> &scale(T x, T y)
        {
            Matrix3<T> trans;
            Mem::zero(trans.m_data, sizeof(T) * 3 * 3);
            trans.m_data[0][0] = x;
            trans.m_data[1][1] = y;
            trans.m_data[2][2] = T(1);

            (*this) *= trans;
            return *this;
        }

        Matrix3<T> &scale(T val)
        {
            Matrix3<T> trans;
            Mem::zero(trans.m_data, sizeof(T) * 3 * 3);
            trans.m_data[0][0] = val;
            trans.m_data[1][1] = val;
            trans.m_data[2][2] = T(1);

            (*this) *= trans;
            return *this;
        }

        Matrix3<T> &scale(const Vector2<T> &v)
        {
            Matrix3<T> trans;
            Mem::zero(trans.m_data, sizeof(T) * 3 * 3);
            trans.m_data[0][0] = v.x();
            trans.m_data[1][1] = v.y();
            trans.m_data[2][2] = T(1);

            (*this) *= trans;
            return *this;
        }

        Matrix3<T> &rotate(T theta)
        {
            Matrix3<T> trans;
            trans.loadIdentity();

            T sint = Math::sin(theta);
            T cost = Math::cos(theta);

            trans.m_data[0][0] = cost;
            trans.m_data[1][0] = -sint;
            trans.m_data[0][1] = sint;
            trans.m_data[1][1] = cost;

            (*this) *= trans;
            return *this;
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

        Matrix3<T> &operator *= (const Matrix3<T> &src) {
            T tmp[3][3];

            for(int x = 0; x < 3; x++) {
                tmp[x][0] = m_data[0][0] * src.m_data[x][0] + m_data[1][0] * src.m_data[x][1] + m_data[2][0] * src.m_data[x][2];
                tmp[x][1] = m_data[0][1] * src.m_data[x][0] + m_data[1][1] * src.m_data[x][1] + m_data[2][1] * src.m_data[x][2];
                tmp[x][2] = m_data[0][2] * src.m_data[x][0] + m_data[1][2] * src.m_data[x][1] + m_data[2][2] * src.m_data[x][2];
            }

            Mem::copy(m_data, tmp, 3 * 3 * sizeof(float));
            return *this;
        }

        Matrix3<T> operator * (const Matrix3<T> &src) const {
            Matrix3<T> ret;

            for(int x = 0; x < 3; x++) {
                ret.m_data[x][0] = m_data[0][0] * src.m_data[x][0] + m_data[1][0] * src.m_data[x][1] + m_data[2][0] * src.m_data[x][2];
                ret.m_data[x][1] = m_data[0][1] * src.m_data[x][0] + m_data[1][1] * src.m_data[x][1] + m_data[2][1] * src.m_data[x][2];
                ret.m_data[x][2] = m_data[0][2] * src.m_data[x][0] + m_data[1][2] * src.m_data[x][1] + m_data[2][2] * src.m_data[x][2];
            }

            return ret;
        }

        Vector2<T> operator * (const Vector2<T> &src) const {
            Vector2<T> ret;
            ret.setX(m_data[0][0] * src.x() + m_data[1][0] * src.y() + m_data[2][0]);
            ret.setY(m_data[0][1] * src.x() + m_data[1][1] * src.y() + m_data[2][1]);

            return ret;
        }

        Vector3<T> operator * (const Vector3<T> &src) const {
            Vector3<T> ret;
            ret.setX(m_data[0][0] * src.x() + m_data[1][0] * src.y() + m_data[2][0] * src.z());
            ret.setY(m_data[0][1] * src.x() + m_data[1][1] * src.y() + m_data[2][1] * src.z());
            ret.setZ(m_data[0][2] * src.x() + m_data[1][2] * src.y() + m_data[2][2] * src.z());

            return ret;
        }

        Matrix3<T> transposed() const
        {
            Matrix3<T> ret;
            M_SET_ROW_OF(&ret, 0, m_data[0][0], m_data[0][1], m_data[0][2]);
            M_SET_ROW_OF(&ret, 1, m_data[1][0], m_data[1][1], m_data[1][2]);
            M_SET_ROW_OF(&ret, 2, m_data[2][0], m_data[2][1], m_data[2][2]);

            return ret;
        }

        Matrix3<T> operator ~ () const
        {
            return transposed();
        }

        Matrix3<T> &transpose()
        {
            *this = transposed();
            return *this;
        }
        
    private:
        T m_data[3][3];
    };

    typedef Matrix3<float> Matrix3f;
    typedef Matrix3<double> Matrix3d;
    typedef Matrix3<int> Matrix3i;
}

#undef M_SET_ROW

