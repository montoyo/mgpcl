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
#include "RefCounter.h"

namespace m
{
    template<typename T, class RefCnt> class SharedPtr
    {
    public:
        SharedPtr()
        {
            m_ptr = nullptr;
            m_refs = nullptr;
        }

        SharedPtr(T *ptr)
        {
            if(ptr == nullptr) {
                m_ptr = nullptr;
                m_refs = nullptr;
            } else {
                m_ptr = ptr;
                m_refs = new RefCnt;
                m_refs->addRef();
            }
        }

        SharedPtr(const SharedPtr<T, RefCnt> &src)
        {
            m_ptr = src.m_ptr;
            m_refs = src.m_refs;

            if(src.m_refs != nullptr)
                m_refs->addRef();
        }

        SharedPtr(SharedPtr<T, RefCnt> &&src)
        {
            m_ptr = src.m_ptr;
            m_refs = src.m_refs; //Don't need to increment; we "steal" its ref
            src.m_refs = nullptr;
        }

        ~SharedPtr()
        {
            if(m_refs != nullptr && m_refs->releaseRef()) {
                delete m_ptr;
                delete m_refs;
            }
        }

        bool isNull() const
        {
            return m_refs == nullptr;
        }

        bool operator ! () const
        {
            return m_refs == nullptr;
        }

        void setNull()
        {
            if(m_refs != nullptr && m_refs->releaseRef()) {
                delete m_ptr;
                delete m_refs;
            }

            m_refs = nullptr;
            m_ptr = nullptr;
        }

        SharedPtr<T, RefCnt> &operator = (T *val)
        {
            if(val != m_ptr) {
                if(m_refs != nullptr && m_refs->releaseRef()) {
                    delete m_ptr;
                    delete m_refs;
                }

                if(val == nullptr) {
                    m_ptr = nullptr;
                    m_refs = nullptr;
                } else {
                    m_ptr = val;
                    m_refs = new RefCnt;
                    m_refs->addRef();
                }
            }

            return *this;
        }

        SharedPtr<T, RefCnt> &operator = (const SharedPtr<T, RefCnt> &src)
        {
            if(src.m_refs != nullptr)
                src.m_refs->addRef();

            if(m_refs != nullptr && m_refs->releaseRef()) {
                delete m_ptr;
                delete m_refs;
            }

            m_ptr = src.m_ptr;
            m_refs = src.m_refs;
            return *this;
        }

        SharedPtr<T, RefCnt> &operator = (SharedPtr<T, RefCnt> &&src)
        {
            if(m_refs != nullptr && m_refs->releaseRef()) {
                delete m_ptr;
                delete m_refs;
            }

            m_ptr = src.m_ptr;
            m_refs = src.m_refs;
            src.m_refs = nullptr; //Steal its ref
            return *this;
        }

        bool operator == (const T *cmp) const
        {
            return m_ptr == cmp;
        }

        bool operator == (const SharedPtr<T, RefCnt> cmp) const
        {
            return m_ptr == cmp.m_ptr;
        }

        bool operator != (const T *cmp) const
        {
            return m_ptr != cmp;
        }

        bool operator != (const SharedPtr<T, RefCnt> cmp) const
        {
            return m_ptr != cmp.m_ptr;
        }

        T *operator -> ()
        {
            return m_ptr;
        }

        const T *operator -> () const
        {
            return m_ptr;
        }

        T &operator * ()
        {
            return *m_ptr;
        }

        const T &operator * () const
        {
            return *m_ptr;
        }

        T *ptr()
        {
            return m_ptr;
        }

        const T *ptr() const
        {
            return m_ptr;
        }

        template<typename OtherT, class OtherRefCnt> friend class SharedPtr;
        template<typename Other> SharedPtr<Other, RefCnt> staticCast()
        {
            SharedPtr<Other, RefCnt> ret;
            ret.m_ptr = static_cast<Other*>(m_ptr);
            ret.m_refs = m_refs;

            if(m_refs != nullptr)
                m_refs->addRef();

            return ret;
        }

        template<typename Other> SharedPtr<Other, RefCnt> reinterpretCast()
        {
            SharedPtr<Other, RefCnt> ret;
            ret.m_ptr = reinterpret_cast<Other*>(m_ptr);
            ret.m_refs = m_refs;

            if(m_refs != nullptr)
                m_refs->addRef();

            return ret;
        }

        template<typename Other> SharedPtr<Other, RefCnt> dynamicCast()
        {
            SharedPtr<Other, RefCnt> ret;
            ret.m_ptr = dynamic_cast<Other*>(m_ptr);
            ret.m_refs = m_refs;

            if(m_refs != nullptr)
                m_refs->addRef();

            return ret;
        }

    private:
        T *m_ptr;
        RefCnt *m_refs;
    };

    template<typename T> using SSharedPtr = SharedPtr<T, RefCounter>;
    template<typename T> using MSharedPtr = SharedPtr<T, AtomicRefCounter>;

}
