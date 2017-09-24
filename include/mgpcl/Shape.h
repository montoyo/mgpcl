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
#include "Ray.h"
#include "Enums.h"
#include "Assert.h"

namespace m
{
    template<typename T> class Shape
    {
    public:
        Shape()
        {
            m_userdata = nullptr;
        }

        virtual ~Shape()
        {
        }

        virtual bool isPointInside(const Vector3<T> &p) const = 0;
        virtual bool intersects(const Ray<T> &ray, T *t = nullptr) const = 0;

        virtual int numIntersections() const
        {
            return 1;
        }

        void setUserdata(void *ud)
        {
            m_userdata = ud;
        }

        void *rawUserdata() const
        {
            return m_userdata;
        }

        template<typename P> P *userdata() const
        {
            return static_cast<P*>(m_userdata);
        }

    protected:
        void *m_userdata;
    };

    template<typename T> class Plane : public Shape<T>
    {
    public:
        Plane()
        {
        }

        Plane(const Vector3<T> &pos, const Vector3<T> &normal) : m_pos(pos), m_normal(normal)
        {
        }

        Plane(T px, T py, T pz, T nx, T ny, T nz) : m_pos(px, py, pz), m_normal(nx, ny, nz)
        {
        }

        Plane(const Vector3<T> &normal) : m_normal(normal)
        {
        }

        Plane(T nx, T ny, T nz) : m_normal(nx, ny, nz)
        {
        }

        Plane(const Vector3<T> &pos, const Vector3<T> &u, const Vector3<T> &v) : m_pos(pos), m_normal(u.cross(v))
        {
        }

        Plane<T> &setPos(const Vector3<T> &pos)
        {
            m_pos = pos;
            return *this;
        }

        Plane<T> &setPos(T x, T y, T z)
        {
            m_pos.set(x, y, z);
            return *this;
        }

        Plane<T> &setNormal(const Vector3<T> &n)
        {
            m_normal = n;
            return *this;
        }

        Plane<T> &setNormal(T x, T y, T z)
        {
            m_normal.set(x, y, z);
            return *this;
        }

        Plane<T> &setNormalFromUV(const Vector3<T> &u, const Vector3<T> &v)
        {
            m_normal = u.cross(v);
            return *this;
        }

        Plane<T> &set(const Vector3<T> &pos, const Vector3<T> &normal)
        {
            m_pos = pos;
            m_normal = normal;
            return *this;
        }

        Plane<T> &set(const Vector3<T> &pos, const Vector3<T> &u, const Vector3<T> &v)
        {
            m_pos = pos;
            m_normal = u.cross(v);
            return *this;
        }

        Plane<T> &set(T px, T py, T pz, T nx, T ny, T nz)
        {
            m_pos.set(px, py, pz);
            m_normal.set(nx, ny, nz);
            return *this;
        }

        const Vector3<T> &pos() const
        {
            return m_pos;
        }

        const Vector3<T> &normal() const
        {
            return m_normal;
        }

        Vector3<T> v() const
        {
            if(m_normal.x() != T(0.0))
                return Vector3<T>(-m_normal.z() / m_normal.x(), T(0.0), T(1.0));
            else if(m_normal.y() != T(0.0))
                return Vector3<T>(T(1.0), -m_normal.x() / m_normal.y(), T(0.0));
            else
                return Vector3<T>(T(0.0), T(1.0), -m_normal.y() / m_normal.z());
        }

        Vector3<T> u() const
        {
            Vector3<T> vec(v());
            return m_normal.cross(vec);
        }

        bool isPointInside(const Vector3<T> &p) const override
        {
            return p.dot(m_normal) == m_pos.dot(m_normal);
        }

        bool intersects(const Ray<T> &ray, T *t = nullptr) const override
        {
            T den = m_normal.dot(ray.p());
            if(den == T(0.0))
                return false;

            if(t != nullptr)
                *t = m_normal.dot(m_pos - ray.s()) / den;

            return true;
        }

    private:
        Vector3<T> m_pos;
        Vector3<T> m_normal;
    };

    template<typename T> class Sphere : public Shape<T>
    {
    public:
        Sphere()
        {
            m_radius = T(0.0);
        }

        Sphere(T radius)
        {
            m_radius = radius;
        }

        Sphere(const Vector3<T> pos, T radius) : m_pos(pos)
        {
            m_radius = radius;
        }

        Sphere(T x, T y, T z, T radius) : m_pos(x, y, z)
        {
            m_radius = radius;
        }

        Sphere<T> &setPos(const Vector3<T> &pos)
        {
            m_pos = pos;
            return *this;
        }

        Sphere<T> &setPos(T x, T y, T z)
        {
            m_pos.set(x, y, z);
            return *this;
        }

        Sphere<T> &setRadius(T radius)
        {
            m_radius = radius;
            return *this;
        }

        Sphere<T> &setDiameter(T diam)
        {
            m_radius = diam / T(2.0);
            return *this;
        }

        Sphere<T> &set(const Vector3<T> &pos, T radius)
        {
            m_pos = pos;
            m_radius = radius;
            return *this;
        }

        Sphere<T> &set(T x, T y, T z, T radius)
        {
            m_pos.set(x, y, z);
            m_radius = radius;
            return *this;
        }

        const Vector3<T> &pos() const
        {
            return m_pos;
        }

        T radius() const
        {
            return m_radius;
        }

        T diameter() const
        {
            return m_radius * T(2.0);
        }

        T volume() const
        {
            //I bet $10,000 this will never be used...
            return T(4.0 * M_PI / 3.0) * m_radius * m_radius * m_radius;
        }

        int numIntersections() const override
        {
            return 2;
        }

        bool isPointInside(const Vector3<T> &p) const override
        {
            return (p - m_pos).length2() <= m_radius * m_radius;
        }

        bool intersects(const Ray<T> &ray_, T *t = nullptr) const override
        {
            Ray<T> ray(ray_ - m_pos);
            T a = ray.p().length2();
            T b = ray.s().dot(ray.p()) * T(2.0);
            T c = ray.s().length2() - m_radius * m_radius;
            T delta = b * b - T(4.0) * a * c;

            if(delta < T(0.0))
                return false;

            if(t != nullptr) {
                if(delta == T(0.0)) {
                    t[0] = -b / (T(2.0) * a);
                    t[1] = t[0];
                } else {
                    T sqd = Math::sqrt<T>(delta);

                    t[0] = (-sqd - b) / (T(2.0) * a);
                    t[1] = ( sqd - b) / (T(2.0) * a);
                }
            }

            return true;
        }

        Vector2<T> unwrap(const Vector3<T> &p)
        {
            Vector3<T> lp(p - m_pos);
            lp /= m_radius;

            mAssert(lp.length2() == T(1.0), "point is not on the sphere");
            return lp.unwrapSphere();
        }

    private:
        Vector3<T> m_pos;
        T m_radius;
    };

    template<typename T> class Cone : public Shape<T>
    {
    public:
        Cone()
        {
            m_axis = Axis::X;
            m_radius = T(0.0);
            m_length = T(0.0);
        }

        Cone(Axis a, T radius, T len)
        {
            m_axis = a;
            m_radius = radius;
            m_length = len;
        }

        Cone(const Vector3<T> &pos, Axis a, T radius, T len) : m_pos(pos)
        {
            m_axis = a;
            m_radius = radius;
            m_length = len;
        }

        Cone(T px, T py, T pz, Axis a, T radius, T len) : m_pos(px, py, pz)
        {
            m_axis = a;
            m_radius = radius;
            m_length = len;
        }

        int numIntersections() const override
        {
            return 2;
        }

        bool isPointInside(const Vector3<T> &p_) const override
        {
            Vector3<T> p(p_ - m_pos);
            reverseAxis(p);

            if(p.x() < 0 || p.x() >= m_length)
                return false;

            T r = T(1.0) - p.x() / m_length;
            return p.y() * p.y() + p.z() * p.z() <= r * r;
        }

        bool intersects(const Ray<T> &ray_, T *t = nullptr) const override
        {
            //Didn't check my math here; hopefully this is ok...
            Ray<T> ray(ray_ - m_pos);
            reverseAxis(ray.s());
            reverseAxis(ray.p());

            Vector3<T> s2(ray.s() * ray.s());
            Vector3<T> p2(ray.p() * ray.p());
            Vector3<T> sp(ray.s() * ray.p());
            T r2 = m_radius * m_radius;
            T h2 = m_length * m_length;

            T a = p2.y() + p2.z() - p2.x() * r2 / h2;
            T b = T(2.0) * (sp.y() + sp.z() + r2 * ray.p().x() / m_length + r2 * sp.x() / h2);
            T c = s2.y() + s2.z() + T(2.0) * r2 * ray.s().x() / m_length - s2.x() * r2 / h2 - r2;
            T delta = b * b - T(4.0) * a * c;

            if(delta < T(0.0))
                return false;

            if(delta == T(0.0)) {
                T t0 = -b / (T(2.0) * a);
                T x0 = ray[t0].x();

                if(x0 < 0 || x0 >= m_length)
                    return false;

                if(t != nullptr) {
                    t[0] = t0;
                    t[1] = t0;
                }

                return true;
            } else {
                T sqd = Math::sqrt<T>(delta);
                T t1 = (-sqd - b) / (T(2.0) * a);
                T t2 = ( sqd - b) / (T(2.0) * a);
                T x1 = ray[t1].x();
                T x2 = ray[t2].x();
                
                if(x1 < 0 || x1 >= m_length) {
                    if(x2 < 0 || x2 >= m_length)
                        return false;

                    t1 = t2; //t1 is invalid; replace by t2
                } else if(x2 < 0 || x2 >= m_length)
                    t2 = t1; //t2 is invalid; replace by t1

                if(t != nullptr) {
                    t[0] = t1;
                    t[1] = t2;
                }

                return true;
            }
        }

        Cone &setPos(const Vector3<T> &pos)
        {
            m_pos = pos;
            return *this;
        }

        Cone &setPos(T x, T y, T z)
        {
            m_pos.set(x, y, z);
            return *this;
        }

        Cone &setAxis(Axis a)
        {
            m_axis = a;
            return *this;
        }

        Cone &setRadius(T radius)
        {
            m_radius = radius;
            return *this;
        }

        Cone &setDiameter(T diam)
        {
            m_radius = diam / T(2.0);
            return *this;
        }

        Cone &setLength(T len)
        {
            m_length = len;
            return *this;
        }

        Cone &set(Axis a, T radius, T len)
        {
            m_axis = a;
            m_radius = radius;
            m_length = len;
            return *this;
        }

        Cone &set(const Vector3<T> &pos, Axis a, T radius, T len)
        {
            m_pos = pos;
            m_axis = a;
            m_radius = radius;
            m_length = len;
            return *this;
        }

        Cone &set(T px, T py, T pz, Axis a, T radius, T len)
        {
            m_pos.set(px, py, pz);
            m_axis = a;
            m_radius = radius;
            m_length = len;
            return *this;
        }

        const Vector3<T> &pos() const
        {
            return m_pos;
        }

        Axis axis() const
        {
            return m_axis;
        }

        T radius() const
        {
            return m_radius;
        }

        T diameter() const
        {
            return m_radius * T(2.0);
        }

        T length() const
        {
            return m_length;
        }

        T volume() const
        {
            //I bet $20,000 this will never be used...
            return T(M_PI / 3.0) * m_radius * m_radius * m_length;
        }

    private:
        void reverseAxis(Vector3<T> &v)
        {
            T tmp;

            switch(m_axis) {
            case Axis::X:
                //Nothing to do!
                break;

            case Axis::Y:
                tmp = v.x();
                v.setX(v.y());
                v.setY(tmp);
                break;

            case Axis::Z:
                tmp = v.x();
                v.setX(v.z());
                v.setZ(tmp);
                break;
            }
        }

        Vector3<T> m_pos;
        Axis m_axis;
        T m_radius;
        T m_length;
    };

    template<typename T> class Triangle : public Shape<T>
    {
    public:
        Triangle()
        {
        }

        Triangle(const Vector3<T> &a, const Vector3<T> &b, const Vector3<T> &c)
        {
            m_v[0] = a;
            m_v[1] = b;
            m_v[2] = c;
        }

        const Vector3<T> &vertex(int i)
        {
            mDebugAssert(i >= 0 && i < 3, "invalid triangle vertex id");
            return m_v[i];
        }

        Triangle &setVertex(int i, const Vector3<T> &v)
        {
            mDebugAssert(i >= 0 && i < 3, "invalid triangle vertex id");
            m_v[i] = v;
            return *this;
        }

        Triangle &setVertex(int i, T x, T y, T z)
        {
            mDebugAssert(i >= 0 && i < 3, "invalid triangle vertex id");
            m_v[i].set(x, y, z);
            return *this;
        }

        Vector3<T> normal() const
        {
            Vector3<T> q1(m_v[1] - m_v[0]);
            Vector3<T> q2(m_v[2] - m_v[0]);

            return q1.cross(q2);
        }

        Vector3<T> barycentricCoords(const Vector3<T> &r)
        {
            return barycentricCoords(m_v[1] - m_v[0], m_v[2] - m_v[0], r - m_v[0]);
        }

        bool isPointInside(const Vector3<T> &p) const override
        {
            Vector3<T> q1(m_v[1] - m_v[0]);
            Vector3<T> q2(m_v[2] - m_v[0]);
            Vector3<T> n(q1.cross(q2));
            if(p.dot(n) != m_v[0].dot(n))
                return false;

            Vector3<T> w(barycentricCoords(q1, q2, p - m_v[0]));
            return w.y() >= T(0.0) && w.z() >= T(0.0) && w.y() + w.z() <= T(1.0);
        }

        bool intersects(const Ray<T> &ray, T *t = nullptr) const override
        {
            Vector3<T> q1(m_v[1] - m_v[0]);
            Vector3<T> q2(m_v[2] - m_v[0]);
            Vector3<T> n(q1.cross(q2));

            T den = n.dot(ray.p());
            if(den == T(0.0))
                return false;

            T t0 = n.dot(m_v[0] - ray.s()) / den;
            Vector3<T> w(barycentricCoords(q1, q2, ray[t0] - m_v[0]));

            if(w.y() < T(0.0) || w.z() < T(0.0) || w.y() + w.z() > T(1.0))
                return false;

            if(t != nullptr)
                *t = t0;

            return true;
        }

    private:
        static Vector3<T> barycentricCoords(const Vector3<T> &q1, const Vector3<T> &q2, const Vector3<T> &r)
        {
            T q1q2 = q1.dot(q2);
            T q12  = q1.length2();
            T q22  = q2.length2();
            T idet = T(1.0) / (q12 * q22 - q1q2 * q1q2);
            T rq1  = r.dot(q1);
            T rq2  = r.dot(q2);

            T w1 = (q22 * rq1 - q1q2 * rq2) * idet;
            T w2 = (q12 * rq2 - q1q2 * rq1) * idet;

            return Vector3<T>(T(1.0) - w1 - w2, w1, w2);
        }

        Vector3<T> m_v[3];
    };

    typedef Plane<float> Planef;
    typedef Plane<double> Planed;
    typedef Sphere<float> Spheref;
    typedef Sphere<double> Sphered;
    typedef Cone<float> Conef;
    typedef Cone<double> Coned;
    typedef Triangle<float> Trianglef;
    typedef Triangle<double> Triangled;

}
