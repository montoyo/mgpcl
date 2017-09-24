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

#include "mgpcl/Window.h"

#ifndef MGPCL_NO_GUI
#include "mgpcl/Mem.h"
#include "mgpcl/Random.h"

#ifdef MGPCL_WIN
#include <Windowsx.h>
#define M_WINDOW_INIT
#else
#include "mgpcl/GUI.h"
#define M_WINDOW_INIT : m_queue(8), m_keyState(kKC_Max)
//Yes i'm lazy...
#endif

m::Window::Window() M_WINDOW_INIT
{
    m_wnd = nullptr;

#ifdef MGPCL_WIN
    m_inst = GetModuleHandle(nullptr);
    m_icon = nullptr;
    m_iconColor = nullptr;
    m_iconMask = nullptr;
    m_resizable = true;
    m_save.fullscreen = false;
    m_save.maximized = false;
#endif
}

m::Window::Window(const String &title, int width, int height) M_WINDOW_INIT
{
    m_wnd = nullptr;

#ifdef MGPCL_WIN
    m_inst = GetModuleHandle(nullptr);
    m_icon = nullptr;
    m_iconColor = nullptr;
    m_iconMask = nullptr;
    m_resizable = true;
    m_save.fullscreen = false;
    m_save.maximized = false;
#endif

    create(title, width, height);
}

m::Window::~Window()
{
#ifdef MGPCL_WIN
    if(m_wnd != nullptr) {
        ShowWindow(m_wnd, SW_HIDE);
        DestroyWindow(m_wnd);
    }

    if(m_icon != nullptr) {
        DestroyIcon(m_icon);
        DeleteBitmap(m_iconMask);
        DeleteBitmap(m_iconColor);
    }
#else
    if(m_wnd != nullptr) {
        gtk_widget_hide(m_wnd);
        gtk_widget_destroy(m_wnd);
        g_object_unref(m_wnd);
    }

    //Required to correctly close the window...
    while(gtk_events_pending())
        gtk_main_iteration();
#endif
}

#ifdef MGPCL_WIN
static void fillModifiers(int &mods, WPARAM wParam)
{
    mods = 0;
    if(wParam & MK_CONTROL)
        mods |= m::kWEM_Control;

    if(wParam & MK_SHIFT)
        mods |= m::kWEM_Shift;

    if(wParam & MK_LBUTTON)
        mods |= m::kWEM_LButton;

    if(wParam & MK_MBUTTON)
        mods |= m::kWEM_MButton;
    
    if(wParam & MK_RBUTTON)
        mods |= m::kWEM_RButton;
}

static void fixScancode(m::ScanCode &sc, m::KeyCode kc)
{
    if(kc >= m::kKC_KP0 && kc <= m::kKC_KP9) {
        int idx = kc - m::kKC_KP0 + m::kSC_KP0;
        sc = static_cast<m::ScanCode>(idx);
        return;
    }

    switch(kc) {
    case m::kKC_KPSlash:
        sc = m::kSC_KPSlash;
        break;

    case m::kKC_KPStar:
        sc = m::kSC_KPStar;
        break;

    case m::kKC_KPMinus:
        sc = m::kSC_KPMinus;
        break;

    case m::kKC_KPPlus:
        sc = m::kSC_KPPlus;
        break;

    case m::kKC_KPEnter:
        sc = m::kSC_KPEnter;
        break;

    case m::kKC_KPPeriod:
        sc = m::kSC_KPPeriod;
        break;

    default:
        break;
    }
}

static m::KeyCode fixKeycode(WPARAM wParam, LPARAM lParam)
{
    if(wParam == VK_RETURN && (lParam & (1 << 24)) != 0)
        return m::kKC_KPEnter;

    return static_cast<m::KeyCode>(wParam);
}

LRESULT CALLBACK m::Window::wndProc(HWND wnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Window *me = reinterpret_cast<Window*>(GetWindowLongPtr(wnd, GWLP_USERDATA));

    switch(uMsg) {
    case WM_CLOSE:
    case WM_DESTROY:
        me->m_event.type = kWET_Closing;
        break;

    case WM_MOUSEMOVE:
        fillModifiers(me->m_event.modifiers, wParam);
        me->m_event.type = kWET_MouseMove;
        me->m_event.xy.set(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_LBUTTONDOWN:
        fillModifiers(me->m_event.modifiers, wParam);
        me->m_event.type = kWET_MousePress;
        me->m_event.button = MouseButton::Left;
        me->m_event.xy.set(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_RBUTTONDOWN:
        fillModifiers(me->m_event.modifiers, wParam);
        me->m_event.type = kWET_MousePress;
        me->m_event.button = MouseButton::Right;
        me->m_event.xy.set(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_MBUTTONDOWN:
        fillModifiers(me->m_event.modifiers, wParam);
        me->m_event.type = kWET_MousePress;
        me->m_event.button = MouseButton::Middle;
        me->m_event.xy.set(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_LBUTTONUP:
        fillModifiers(me->m_event.modifiers, wParam);
        me->m_event.type = kWET_MouseRelease;
        me->m_event.button = MouseButton::Left;
        me->m_event.xy.set(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_RBUTTONUP:
        fillModifiers(me->m_event.modifiers, wParam);
        me->m_event.type = kWET_MouseRelease;
        me->m_event.button = MouseButton::Right;
        me->m_event.xy.set(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_MBUTTONUP:
        fillModifiers(me->m_event.modifiers, wParam);
        me->m_event.type = kWET_MouseRelease;
        me->m_event.button = MouseButton::Middle;
        me->m_event.xy.set(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_MOUSEWHEEL:
        fillModifiers(me->m_event.modifiers, LOWORD(wParam));
        me->m_event.type = kWET_MouseScroll;
        me->m_event.xy.set(0, HIWORD(wParam));
        break;

    case WM_MOUSEHWHEEL:
        fillModifiers(me->m_event.modifiers, LOWORD(wParam));
        me->m_event.type = kWET_MouseScroll;
        me->m_event.xy.set(HIWORD(wParam), 0);
        break;

    case WM_KEYDOWN:
        me->m_event.type = kWET_KeyDown;
        me->m_event.keycode = fixKeycode(wParam, lParam);
        me->m_event.scancode = static_cast<ScanCode>((lParam & 0x00FF0000) >> 16);
        fixScancode(me->m_event.scancode, me->m_event.keycode);
        me->m_event.isRepeat = (lParam & 0x40000000) != 0;
        break;

    case WM_KEYUP:
        me->m_event.type = kWET_KeyUp;
        me->m_event.keycode = fixKeycode(wParam, lParam);
        me->m_event.scancode = static_cast<ScanCode>((lParam & 0x00FF0000) >> 16);
        fixScancode(me->m_event.scancode, me->m_event.keycode);
        break;

    case WM_CHAR:
    {
        wchar_t str[2] = { static_cast<wchar_t>(wParam), 0 };
        int conv = WideCharToMultiByte(CP_UTF8, 0, str, 2, me->m_event.text, 8, nullptr, nullptr);

        if(conv > 0 && conv <= 8) {
            me->m_event.type = kWET_KeyType;
            me->m_event.isRepeat = (lParam & 0x40000000) != 0;
        }
        break;
    }

    case WM_SIZE:
        if(wParam == SIZE_MAXIMIZED)
            me->m_event.type = kWET_Maximize;
        else if(wParam == SIZE_MINIMIZED)
            me->m_event.type = kWET_Minimize;

        if(!me->m_save.fullscreen) {
            if(wParam == SIZE_MAXIMIZED)
                me->m_save.maximized = true;
            else if(wParam == SIZE_RESTORED)
                me->m_save.maximized = false;
        }

        break;

    case WM_KILLFOCUS:
        me->m_event.type = kWET_LooseFocus;
        break;

    case WM_SETFOCUS:
        me->m_event.type = kWET_GainFocus;
        break;

    default:
        return DefWindowProc(wnd, uMsg, wParam, lParam);
    }

    return 0;
}
#else
static void fillModifiers(int &mods, guint state)
{
    mods = 0;
    if(state & GDK_CONTROL_MASK)
        mods |= m::kWEM_Control;

    if(state & GDK_SHIFT_MASK)
        mods |= m::kWEM_Shift;

    if(state & GDK_BUTTON1_MASK)
        mods |= m::kWEM_LButton;

    if(state & GDK_BUTTON2_MASK)
        mods |= m::kWEM_MButton;

    if(state & GDK_BUTTON3_MASK)
        mods |= m::kWEM_RButton;
}

static m::MouseButton toMButton(guint btn)
{
    if(btn == 1)
        return m::MouseButton::Left;
    else if(btn == 2)
        return m::MouseButton::Middle;
    else if(btn == 3)
        return m::MouseButton::Right;

    return m::MouseButton::Left; //TODO: FIXME: FIX THIS!
}

static void fillRepeat(bool &isRep, m::KeyCode kc, m::Bitfield &bf, bool state)
{
    if(kc >= m::kKC_Max)
        isRep = false;
    else {
        if(bf.bit(kc)) {
            isRep = true;

            if(!state)
                bf.clearBit(kc);
        } else {
            isRep = false;

            if(state)
                bf.setBit(kc);
        }
    }
}

gboolean m::Window::wndProc(GtkWidget *wnd, GdkEvent *gev, gpointer ud)
{
    Window *me = static_cast<Window*>(ud);
    WindowEvent ev;

    switch(gev->type) {
    case GDK_DELETE:
    case GDK_DESTROY:
        ev.type = kWET_Closing;
        break;

    case GDK_FOCUS_CHANGE:
        if(gev->focus_change.in)
            ev.type = kWET_GainFocus;
        else
            ev.type = kWET_LooseFocus;

        break;

    case GDK_MOTION_NOTIFY:
        ev.type = kWET_MouseMove;
        fillModifiers(ev.modifiers, gev->motion.state);
        ev.xy.set(static_cast<int>(gev->motion.x), static_cast<int>(gev->motion.y));
        break;

    case GDK_BUTTON_PRESS:
        ev.type = kWET_MousePress;
        fillModifiers(ev.modifiers, gev->button.state);
        ev.xy.set(static_cast<int>(gev->button.x), static_cast<int>(gev->button.y));
        ev.button = toMButton(gev->button.button);
        break;

    case GDK_BUTTON_RELEASE:
        ev.type = kWET_MouseRelease;
        fillModifiers(ev.modifiers, gev->button.state);
        ev.xy.set(static_cast<int>(gev->button.x), static_cast<int>(gev->button.y));
        ev.button = toMButton(gev->button.button);
        break;

    case GDK_SCROLL:
        ev.type = kWET_MouseScroll;
        ev.xy.set(static_cast<int>(gev->scroll.delta_x * 120.0), static_cast<int>(gev->scroll.delta_y * 120.0));
        break;

    case GDK_WINDOW_STATE:
    {
        GdkWindowState changed = gev->window_state.changed_mask;
        GdkWindowState flags = gev->window_state.new_window_state;

        if(changed & (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_MAXIMIZED)) {
            if(flags & GDK_WINDOW_STATE_ICONIFIED)
                ev.type = kWET_Minimize;
            else if(flags & GDK_WINDOW_STATE_MAXIMIZED)
                ev.type = kWET_Maximize;
        }
        break;
    }

    case GDK_KEY_PRESS:
        ev.type = kWET_KeyDown;
        ev.scancode = keyboard::remapScancode(gev->key.hardware_keycode);
        ev.keycode = keyboard::remapKeycode(gev->key.keyval);
        fillRepeat(ev.isRepeat, ev.keycode, me->m_keyState, true);

        if(gev->key.length > 0 && gev->key.length < 7) {
            me->m_queue.offer(ev);

            ev.type = kWET_KeyType;
            Mem::copy(ev.text, gev->key.string, gev->key.length);
            ev.text[gev->key.length] = 0;
            //ev.isRepeat shouldn't change, so that's good!
        }

        break;

    case GDK_KEY_RELEASE:
        ev.type = kWET_KeyUp;
        ev.scancode = keyboard::remapScancode(gev->key.hardware_keycode);
        ev.keycode = keyboard::remapKeycode(gev->key.keyval);
        //fillRepeat(ev.isRepeat, ev.keycode, me->m_keyState, false);
        break;

    default:
        break;
    }

    me->m_queue.offer(ev);
    return FALSE;
}
#endif

bool m::Window::create(const String &title, int width, int height)
{
#ifdef MGPCL_WIN
    if(m_wnd != nullptr) {
        ShowWindow(m_wnd, SW_HIDE);
        DestroyWindow(m_wnd);
        m_wnd = nullptr;
    }

    if(m_icon != nullptr) {
        DestroyIcon(m_icon);
        DeleteBitmap(m_iconMask);
        DeleteBitmap(m_iconColor);

        m_iconColor = nullptr;
        m_iconMask = nullptr;
        m_icon = nullptr;
    }

    WNDCLASSEX wndCls;
    Mem::zero(wndCls);

    wndCls.cbSize = sizeof(WNDCLASSEX);
    wndCls.hInstance = m_inst;
    wndCls.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndCls.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wndCls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND + 1);
    wndCls.lpfnWndProc = wndProc;

    Random<> rnd;
    m_cls = "MGPCL_";

    uint8_t rndBytes[4];
    rnd.nextBytes(rndBytes, 4);
    hexString(rndBytes, 4, m_cls);

    wndCls.lpszClassName = m_cls.raw();

    if(RegisterClassEx(&wndCls) == 0)
        return false;

    m_wnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, m_cls.raw(), title.raw(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, m_inst, nullptr);
    if(m_wnd == nullptr)
        return false;

    SetWindowLongPtr(m_wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    return true;
#else
    if(!m::gui::hasBeenInitialized())
        m::gui::initialize();

    if(m_wnd != nullptr) {
        gtk_widget_hide(m_wnd);
        gtk_widget_destroy(m_wnd);
        g_object_unref(m_wnd);
        m_wnd = nullptr;
    }

    m_keyState.clear();
    m_wnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(m_wnd);
    gtk_window_set_default_size(GTK_WINDOW(m_wnd), width, height);
    g_signal_connect(m_wnd, "event", G_CALLBACK(wndProc), this);
    return true;
#endif
}

bool m::Window::pollEvent()
{
    mDebugAssert(m_wnd != nullptr, "window wasn't created");

#ifdef MGPCL_WIN
    m_event.clear();

    MSG msg;
    while(!m_event.isValid() && PeekMessage(&msg, m_wnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#else
    while(m_queue.isEmpty() && gtk_events_pending())
        gtk_main_iteration();

    if(m_queue.isEmpty())
        m_event.clear();
    else {
        m_event = m_queue.first();
        m_queue.poll();
    }
#endif

    return m_event.isValid();
}

void m::Window::waitEvent()
{
    mDebugAssert(m_wnd != nullptr, "window wasn't created");

#ifdef MGPCL_WIN
    MSG msg;

    m_event.clear();
    while(!!m_event) {
        if(GetMessage(&msg, m_wnd, 0, 0)) {
            //GetMessage only fails if the window was closed anyway
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
#else
    while(m_queue.isEmpty())
        gtk_main_iteration(); //gtk_main_interation() sleeps while the event queue is empty...

    m_event = m_queue.first();
    m_queue.poll();
#endif
}

void m::Window::show(bool shown)
{
    mDebugAssert(m_wnd != nullptr, "window wasn't created");

#ifdef MGPCL_WIN
    ShowWindow(m_wnd, shown ? SW_SHOW : SW_HIDE);
#else
    if(shown)
        gtk_widget_show_all(m_wnd);
    else
        gtk_widget_hide(m_wnd);
#endif
}

void m::Window::setTitle(const String &title)
{
    mDebugAssert(m_wnd != nullptr, "window wasn't created");

#ifdef MGPCL_WIN
    SetWindowText(m_wnd, title.raw());
#else
    gtk_window_set_title(GTK_WINDOW(m_wnd), title.raw());
#endif
}

m::Vector2i m::Window::pos() const
{
    mDebugAssert(m_wnd != nullptr, "window wasn't created");

#ifdef MGPCL_WIN
    RECT rect;
    GetWindowRect(m_wnd, &rect);
    return Vector2i(rect.left, rect.top);
#else
    gint px, py;
    gtk_window_get_position(GTK_WINDOW(m_wnd), &px, &py);

    return Vector2i(px, py);
#endif
}

m::Vector2i m::Window::size() const
{
    mDebugAssert(m_wnd != nullptr, "window wasn't created");

#ifdef MGPCL_WIN
    RECT rect;
    GetWindowRect(m_wnd, &rect);
    return Vector2i(rect.right - rect.left, rect.bottom - rect.top);
#else
    gint w, h;
    gtk_window_get_size(GTK_WINDOW(m_wnd), &w, &h);

    return Vector2i(w, h);
#endif
}

void m::Window::setIcon(const uint8_t *pixels, int width, int height)
{
    mDebugAssert(m_wnd != nullptr, "window wasn't created");

#ifdef MGPCL_WIN
    HBITMAP color = CreateBitmap(width, height, 1, 32, pixels);
    HBITMAP mask = CreateCompatibleBitmap(GetDC(nullptr), width, height);

    ICONINFO ii;
    Mem::zero(ii);
    ii.fIcon = TRUE;
    ii.hbmColor = color;
    ii.hbmMask = mask;

    HICON icon = CreateIconIndirect(&ii);
    SetClassLongPtr(m_wnd, GCLP_HICON, reinterpret_cast<LONG_PTR>(icon));

    if(m_icon != nullptr) {
        DestroyIcon(m_icon);
        DeleteBitmap(m_iconMask);
        DeleteBitmap(m_iconColor);
    }

    m_iconColor = color;
    m_iconMask = mask;
    m_icon = icon;
#else
    GBytes *gpixels = g_bytes_new(pixels, static_cast<gsize>(width * height * 4));
    GdkPixbuf *icon = gdk_pixbuf_new_from_bytes(gpixels, GDK_COLORSPACE_RGB, TRUE, 8, width, height, width * 4);
    g_object_ref_sink(icon);
    gtk_window_set_icon(GTK_WINDOW(m_wnd), icon);
    g_object_unref(icon); //Hopefully the window added its ref!
    g_bytes_unref(gpixels); //Hopefully GdkPixbuf added its ref!
#endif
}

void m::Window::destroy()
{
#ifdef MGPCL_WIN
    if(m_wnd != nullptr) {
        ShowWindow(m_wnd, SW_HIDE);
        DestroyWindow(m_wnd);
        m_wnd = nullptr;
    }
#else
    if(m_wnd != nullptr) {
        gtk_widget_hide(m_wnd);
        gtk_widget_destroy(m_wnd);
        g_object_unref(m_wnd);
        m_wnd = nullptr;
    }

    //Required to correctly close the window...
    while(gtk_events_pending())
        gtk_main_iteration();
#endif
}

void m::Window::setResizable(bool rs)
{
    mDebugAssert(m_wnd != nullptr, "window wasn't created");

#ifdef MGPCL_WIN
    LONG_PTR style = GetWindowLongPtr(m_wnd, GWL_STYLE);

    if(rs)
        style |= WS_THICKFRAME;
    else
        style &= ~static_cast<LONG_PTR>(WS_THICKFRAME);

    SetWindowLongPtr(m_wnd, GWL_STYLE, style);
    m_resizable = rs;
#else
    gtk_window_set_resizable(GTK_WINDOW(m_wnd), rs ? TRUE : FALSE);
#endif
}

void m::Window::move(int x, int y)
{
    mDebugAssert(m_wnd != nullptr, "window wasn't created");

#ifdef MGPCL_WIN
    SetWindowPos(m_wnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
#else
    gtk_window_move(GTK_WINDOW(m_wnd), x, y);
#endif
}

void m::Window::resize(int w, int h)
{
    mDebugAssert(m_wnd != nullptr, "window wasn't created");

#ifdef MGPCL_WIN
    SetWindowPos(m_wnd, nullptr, 0, 0, w, h, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
#else
    gtk_window_set_default_size(GTK_WINDOW(m_wnd), w, h);
    gtk_window_resize(GTK_WINDOW(m_wnd), w, h);
#endif
}

void m::Window::setBorderless(bool bl)
{
    mDebugAssert(m_wnd != nullptr, "window wasn't created");

#ifdef MGPCL_WIN
    if(bl) {
        SetWindowLongPtr(m_wnd, GWL_STYLE, WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE);
        SetWindowLongPtr(m_wnd, GWL_EXSTYLE, 0);
    } else {
        LONG_PTR style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        if(!m_resizable)
            style &= ~static_cast<LONG_PTR>(WS_THICKFRAME);

        SetWindowLongPtr(m_wnd, GWL_STYLE, style);
        SetWindowLongPtr(m_wnd, GWL_EXSTYLE, WS_EX_OVERLAPPEDWINDOW);
    }
#else
    gtk_window_set_decorated(GTK_WINDOW(m_wnd), bl ? FALSE : TRUE);
#endif
}

void m::Window::setFullscreen(bool fs)
{
    mDebugAssert(m_wnd != nullptr, "window wasn't created");

#ifdef MGPCL_WIN
    if(fs) {
        if(m_save.fullscreen)
            return;

        m_save.fullscreen = true;

        //Make a backup of the window state
        if(m_save.maximized)
            SendMessage(m_wnd, WM_SYSCOMMAND, SC_RESTORE, 0);

        m_save.style = GetWindowLongPtr(m_wnd, GWL_STYLE);
        m_save.styleEx = GetWindowLongPtr(m_wnd, GWL_EXSTYLE);
        GetWindowRect(m_wnd, &m_save.rect);

        //Go fullscreen
        SetWindowLongPtr(m_wnd, GWL_STYLE, WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE);
        SetWindowLongPtr(m_wnd, GWL_EXSTYLE, 0);

        MONITORINFO mon;
        Mem::zero(mon);
        mon.cbSize = sizeof(MONITORINFO);

        HMONITOR hMon = MonitorFromWindow(m_wnd, MONITOR_DEFAULTTONEAREST);
        GetMonitorInfo(hMon, &mon);

        WindowRect rect(mon.rcMonitor.left, mon.rcMonitor.top, mon.rcMonitor.right - mon.rcMonitor.left, mon.rcMonitor.bottom - mon.rcMonitor.top);
        SetWindowPos(m_wnd, nullptr, rect.x, rect.y, rect.w, rect.h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
    } else if(m_save.fullscreen) {
        SetWindowLongPtr(m_wnd, GWL_STYLE, m_save.style);
        SetWindowLongPtr(m_wnd, GWL_EXSTYLE, m_save.styleEx);

        WindowRect rect(m_save.rect.left, m_save.rect.top, m_save.rect.right - m_save.rect.left, m_save.rect.bottom - m_save.rect.top);
        SetWindowPos(m_wnd, nullptr, rect.x, rect.y, rect.w, rect.h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

        if(m_save.maximized)
            SendMessage(m_wnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);

        m_save.fullscreen = false;
    }
#else
    if(fs)
        gtk_window_fullscreen(GTK_WINDOW(m_wnd));
    else
        gtk_window_unfullscreen(GTK_WINDOW(m_wnd));
#endif
}

#ifdef MGPCL_WIN
static BOOL CALLBACK monitorEnum(HMONITOR mon, HDC hdc, LPRECT rect, LPARAM ud)
{
    int w = rect->right - rect->left;
    int h = rect->bottom - rect->top;

    reinterpret_cast<m::List<m::WindowRect>*>(ud)->add(m::WindowRect(rect->left, rect->top, w, h));
    return TRUE;
}
#endif

void m::Window::getMonitorsGeometry(List<WindowRect> &monitors)
{
#ifdef MGPCL_WIN
    EnumDisplayMonitors(nullptr, nullptr, monitorEnum, reinterpret_cast<LPARAM>(&monitors));
#else
    GdkDisplay *display = gdk_display_get_default();
    const int cnt = gdk_display_get_n_monitors(display);

    for(int i = 0; i < cnt; i++) {
        GdkRectangle rect;
        GdkMonitor *mon = gdk_display_get_monitor(display, i);
        gdk_monitor_get_geometry(mon, &rect);

        monitors.add(WindowRect(rect.x, rect.y, rect.width, rect.height));
    }
#endif
}

#endif
