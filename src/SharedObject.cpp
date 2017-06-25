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

#include "mgpcl/SharedObject.h"

#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

m::SharedObject::SharedObject()
{
    m_so = nullptr;
}

m::SharedObject::SharedObject(const String &name)
{
    load(name);
}

m::SharedObject::~SharedObject()
{
    if(m_so != nullptr) {
#ifdef MGPCL_WIN
        FreeLibrary(static_cast<HMODULE>(m_so));
#else
        dlclose(m_so);
#endif
    }
}

bool m::SharedObject::load(const String &name)
{
#ifdef MGPCL_WIN
    m_so = LoadLibrary(name.raw());
#else
    m_so = dlopen(name.raw(), RTLD_LAZY);
#endif

    return m_so != nullptr;
}

void *m::SharedObject::_addr(const char *name) const
{
#ifdef MGPCL_WIN
    return GetProcAddress(static_cast<HMODULE>(m_so), name);
#else
    return dlsym(m_so, name);
#endif
}

