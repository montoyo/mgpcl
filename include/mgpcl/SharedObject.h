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
#include "String.h"
#include <type_traits>

namespace m
{
    class SharedObject
    {
    public:
        SharedObject();
        SharedObject(const String &name);
        ~SharedObject();

        bool load(const String &name);

        template<typename T> T addressOf(const char *fname) const
        {
            static_assert(std::is_pointer<T>::value, "expected function pointer");
            return reinterpret_cast<T>(_addr(fname));
        }

        template<typename T> T addressOf(const String &fname) const
        {
            static_assert(std::is_pointer<T>::value, "expected function pointer");
            return reinterpret_cast<T>(_addr(fname.raw()));
        }

        bool isOpen() const
        {
            return m_so != nullptr;
        }

    private:
        void *_addr(const char *name) const;
        void *m_so;
    };
}

