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
#include <new>
#include <type_traits>
#include <cstring>
#include <cstdint>

namespace m
{
    namespace mem
    {
        inline void *copy(void *dst, const void *src, size_t sz)
        {
            return memcpy(dst, src, sz);
        }

        inline void *move(void *dst, const void *src, size_t sz)
        {
            return memmove(dst, src, sz);
        }

        inline int cmp(const void *a, const void *b, size_t sz)
        {
            return memcmp(a, b, sz);
        }

        inline void *zero(void *dst, size_t sz)
        {
            return memset(dst, 0, sz);
        }

        template<typename T> T &zero(T &obj)
        {
            return *static_cast<T*>(memset(&obj, 0, sizeof(T)));
        }

        //Only allocate; does not call constructors.
        template<typename T> T *alloc(size_t cnt)
        {
            return reinterpret_cast<T*>(new uint8_t[sizeof(T) * cnt]);
        }

        //Only frees allocated memory; does not call destructors
        template<typename T> void del(T *ptr)
        {
            delete[] reinterpret_cast<uint8_t*>(ptr);
        }

        //Allocates 'cnt' Ts. The returned pointer will be 'align'-bytes aligned.
        //Just like alloc(), no constructor will be called. Use alignedDelete to delete.
        template<typename T> T *alignedNew(size_t cnt, size_t align)
        {
            if(align <= 0 || align > 0xFF)
                throw std::bad_alloc();

            uint8_t *ptr = new uint8_t[sizeof(T) * cnt + align];
            size_t offset = align - reinterpret_cast<size_t>(ptr) % align;

            //Write offset information
            T *ret = reinterpret_cast<T*>(ptr + offset);
            offset--;
            ptr[offset] = static_cast<uint8_t>(offset);

            return ret;
        }

        //Deletes data allocated by alignedNew() and ONLY alignedNew().
        //Just like del(), it won't call the destructor.
        template<typename T> void alignedDelete(T *ptr)
        {
            uint8_t *toDel = reinterpret_cast<uint8_t*>(ptr) - 1;
            toDel -= *toDel;
            delete[] toDel;
        }

        //Cleverly copies an array of cnt Ts into another
        template<typename T> T *copyT(T *dst, const T *src, size_t cnt)
        {
            if(std::is_trivially_copyable<T>::value)
                memcpy(dst, src, cnt * sizeof(T));
            else {
                for(size_t i = 0; i < cnt; i++)
                    dst[i] = src[i];
            }

            return dst;
        }

        //Cleverly initializes an array of cnt Ts from another (calls copy constructor)
        //dst must NOT be intialized (i.e. allocated with mem::alloc)
        template<typename T> T *copyInitT(T *dst, const T *src, size_t cnt)
        {
            if(std::is_trivially_copy_constructible<T>::value)
                memcpy(dst, src, cnt * sizeof(T));
            else {
                for(size_t i = 0; i < cnt; i++)
                    new(dst + i) T(src[i]);
            }

            return dst;
        }

    }
}
