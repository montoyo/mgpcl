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
#include "Config.h"
#include "Enums.h"
#include "Vector2.h"
#include "Keyboard.h"
#include "List.h"

#ifdef MGPCL_LINUX
#include "Bitfield.h"
#include "Queue.h"
#endif

#ifndef MGPCL_NO_GUI
#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#endif

namespace m
{
    enum WindowEventType
    {
        kWET_NoEvent = 0,
        kWET_MouseMove,        //modifiers, xy
        kWET_MousePress,    //modifiers, button, xy
        kWET_MouseRelease,    //modifiers, button, xy
        kWET_MouseScroll,    //xy
        kWET_KeyUp,            //keycode, scancode
        kWET_KeyDown,        //keycode, scancode, isRepeat
        kWET_KeyType,        //text, isRepeat
        kWET_Closing,        //0
        kWET_Minimize,        //0
        kWET_Maximize,        //0
        kWET_GainFocus,        //0
        kWET_LooseFocus        //0
    };

    enum WindowEventModifier
    {
        kWEM_Control = 1,
        kWEM_Shift = 2,
        kWEM_LButton = 4,
        kWEM_MButton = 8,
        kWEM_RButton = 16
    };

    class WindowEvent
    {
    public:
        WindowEvent()
        {
            type = kWET_NoEvent;
        }

        WindowEventType type;
        MouseButton button;
        Vector2i xy;
        ScanCode scancode;
        KeyCode keycode;
        char text[8];
        int modifiers;
        bool isRepeat;

        void clear()
        {
            type = kWET_NoEvent;
        }

        bool isValid() const
        {
            return type != kWET_NoEvent;
        }

        bool operator ! () const
        {
            return type == kWET_NoEvent;
        }

        bool isMouse() const
        {
            return type == kWET_MouseMove || type == kWET_MousePress || type == kWET_MouseRelease;
        }

        bool isKey() const
        {
            return type == kWET_KeyUp || type == kWET_KeyDown;
        }

        bool buttonState() const
        {
            return type == kWET_MousePress;
        }

        bool keyState() const
        {
            return type == kWET_KeyDown;
        }

        bool hasControlModifier() const
        {
            return (modifiers & kWEM_Control) != 0;
        }

        bool hasShiftModifier() const
        {
            return (modifiers & kWEM_Shift) != 0;
        }
    };

    class WindowRect
    {
    public:
        int x;
        int y;
        int w;
        int h;

        WindowRect()
        {
            x = 0;
            y = 0;
            w = 0;
            h = 0;
        }

        WindowRect(int rx, int ry, int rw, int rh)
        {
            x = rx;
            y = ry;
            w = rw;
            h = rh;
        }

        Vector2i pos() const
        {
            return Vector2i(x, y);
        }

        Vector2i size() const
        {
            return Vector2i(w, h);
        }
    };

    class Window
    {
    public:
        Window();
        Window(const String &title, int width, int height);
        ~Window();

        bool create(const String &title, int width, int height);
        void show(bool shown = true);
        void setTitle(const String &title);
        void setIcon(const uint8_t *pixels, int width, int height); //32 bpp, RGBA, so pixels should contain width * height * 4 bytes!
        void move(int x, int y);
        void resize(int w, int h);
        Vector2i pos() const;
        Vector2i size() const;
        void destroy();
        bool pollEvent();
        void waitEvent();
        void setResizable(bool rs = true);
        void setBorderless(bool bl = true);
        void setFullscreen(bool fs = true);

        WindowEvent &lastEvent()
        {
            return m_event;
        }

#if defined(MGPCL_WIN)
        HINSTANCE win32Instance()
        {
            return m_inst;
        }

        HWND win32Wnd()
        {
            return m_wnd;
        }
#elif defined(GDK_WINDOWING_X11)
        ::Window x11Window()
        {
            return gdk_x11_window_get_xid(gtk_widget_get_window(m_wnd));
        }

        ::Display *x11Display()
        {
            GdkScreen *scr = gtk_window_get_screen(GTK_WINDOW(m_wnd));
            return gdk_x11_display_get_xdisplay(gdk_screen_get_display(scr));
        }
#endif

        static void getMonitorsGeometry(List<WindowRect> &monitors);

    private:
#ifdef MGPCL_WIN
        static LRESULT CALLBACK wndProc(HWND wnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        String m_cls;
        HINSTANCE m_inst;
        HWND m_wnd;
        HBITMAP m_iconColor;
        HBITMAP m_iconMask;
        HICON m_icon;
        bool m_resizable;

        struct
        {
            bool fullscreen;
            bool maximized;
            RECT rect;
            LONG_PTR style;
            LONG_PTR styleEx;
        } m_save;
#else
        static gboolean wndProc(GtkWidget *wnd, GdkEvent *ev, gpointer ud);

        GtkWidget *m_wnd;
        Queue<WindowEvent> m_queue;
        Bitfield m_keyState;
#endif

        WindowEvent m_event;
    };
}

#endif
