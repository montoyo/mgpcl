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

#include "mgpcl/GUI.h"
#include "mgpcl/Config.h"

#if defined(MGPCL_NO_GUI)

void m::gui::initialize()
{
}

bool m::gui::hasBeenInitialized()
{
    return false;
}

#elif defined(MGPCL_WIN)

void m::gui::initialize()
{
}

bool m::gui::hasBeenInitialized()
{
    return true;
}

#else
#include "mgpcl/Mutex.h"
#include <gtk/gtk.h>

static volatile bool g_guiInit = false;
static m::Mutex g_guiLock;

void m::gui::initialize()
{
    g_guiLock.lock();
    gtk_init(nullptr, nullptr);
    g_guiInit = true;
    g_guiLock.unlock();
}

bool m::gui::hasBeenInitialized()
{
    volatile bool ret;

    g_guiLock.lock();
    ret = g_guiInit;
    g_guiLock.unlock();

    return ret;
}

#endif
