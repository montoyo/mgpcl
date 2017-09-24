#include <mgpcl/Config.h>

#ifndef MGPCL_NO_GUI
#include "TestAPI.h"
#include <mgpcl/MsgBox.h>
#include <mgpcl/Window.h>
#include <mgpcl/Time.h>
#include <mgpcl/Atomic.h>
#include <mgpcl/Thread.h>

static const struct
{
    int              width;
    int              height;
    int              bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char    pixel_data[16 * 16 * 4 + 1];
} g_iconData = {
    16, 16, 4,
    "\0\0\0\0\0\0\0\0\2278\200\6\2278\200\27\2268\177\27\2073r\6\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0b%S\2\2247~\25\2278\200\27\2278\200\11\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\2329\202:\2319\201\350\2329\202\354\2125uV\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\2103s5\2309\201\347\2329\202\354\232:\202V\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\235<\205>\235<\205\376\233:\203\363\2238}\317\0\0\0"
    "\0\0\0\0\0\0\1\0\0\0\0\0\0\2228||\2269~\337\237<\207\377\235<\205]\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\240;\210>\240;\210\377\234;\205\261\242<\211\377"
    "\2065t^\0\0\0\0\0\0\0\0\0\0\0\0\237<\207\377\2168y?\241<\211\377\240;\210"
    "]\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\246?\215>\246?\215\377\241<\211|\244>\213"
    "\335\243=\212\266\0\0\0\0\0\0\0\0\242=\212\221\245>\214\376\2137v\30\246"
    "?\215\377\246?\215]\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\252A\220>\252?\220\377"
    "\251A\220k\245>\214\252\247?\216\352|3i\30D*@\6\244>\213\312\246?\215\321"
    "\0\0\0\0\251?\217\377\252@\220]\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\255A\223"
    ">\255B\223\377\256B\224m\216<zE\256B\224\377\253A\221M\250@\216\"\256B\224"
    "\366\244?\213\224\0\0\0\0\256B\223\377\256B\224]\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\262C\227>\262C\227\377\261B\226p\0\0\0\0\247A\217\277\232@\204\273"
    "\267D\234?\255B\223\377$'+\3\0\0\0\0\262C\227\377\262C\227]\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\254@\222>\254A\222\377\254@\222p\0\0\0\0\242?\211R\261"
    "B\226\377\236>\206\311\231>\202\213\0\0\0\0\0\0\0\0\254A\222\377\255A\222"
    "]\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\2226|>\2226|\377\2227|p\0\0\0\0\1770k\37"
    "\2206y\361\2227{\377\2011nB\0\0\0\0\0\0\0\0\2226|\377\2227|]\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\232:\203>\232:\203\377\233:\203o\0\0\0\0\0\0\0\7\207"
    "3s\207\2165x\240a$R\24\3\1\2\3\0\0\0\0\232:\203\377\232:\203]\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\242<\211>\243<\212\377\2268\201x\0\0\0""1\0\0\0V\0\0"
    "\0f\0\0\0i\0\0\0_\0\0\0C\0\0\0\0\242=\212\377\243=\212]\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\252@\2170\246?\215\316s*`\203\0\0\0k\0\0\0\235\0\0\0\302\0"
    "\0\0\317\0\0\0\262\0\0\0\211\0\0\0:\250?\216\346\245>\214J\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\17\5\16\2\4\1\3#\0\0\0O\0\0\0y\0\0\0\242\0\0\0\307\0\0"
    "\0\325\0\0\0\273\0\0\0\225\0\0\0m\3\1\3A\3\1\2\20\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\1\0\0\0\31\0\0\0>\0\0\0^\0\0\0y\0\0\0\215\0\0\0\224\0\0\0"
    "\213\0\0\0w\0\0\0Y\0\0\0""8\0\0\0\16\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\3\0\0\0\32\0\0\0""2\0\0\0D\0\0\0P\0\0\0T\0\0\0O\0\0\0E\0\0\0""2"
    "\0\0\0\31\0\0\0\1\0\0\0\0\0\0\0\0"
};

Declare Test("gui"), Priority(13.0);

TEST
{
    volatile StackIntegrityChecker sic;

#ifdef MGPCL_WIN
    m::Atomic running(1);
    int dlgCount = 0;

    m::FunctionalThread thread([&running, &dlgCount] ()
    {
        while(running.get()) {
            m::time::sleepMs(100);
            HWND wnd = FindWindow(nullptr, "This is the title");

            if(wnd != nullptr) {
                if(EndDialog(wnd, 0) != FALSE)
                    dlgCount++;
            }
        }
    }, "DialogKiller2000");

    thread.start();
#endif

    m::msgBox::info("This is the text", "This is the title", m::kMBB_OkCancel);
    m::msgBox::warning("This is the text", "This is the title", m::kMBB_YesNo);
    m::msgBox::error("This is the text", "This is the title", m::kMBB_CancelTryAgainContinue);
    m::msgBox::question("This is the text", "This is the title", m::kMBB_AbortRetryIgnore);
    
#ifdef MGPCL_WIN
    running.set(0);
    thread.join();
    testAssert(dlgCount == 4, "closed an invalid amount of dialogs!");
#endif

    return true;
}

static char unescape(char c)
{
    if(c == '\r')
        return 'r';
    else if(c == '\n')
        return 'n';
    else if(c == '\t')
        return 't';
    else if(c == '\b')
        return 'b';
    else
        return '?';
}

TEST
{
    volatile StackIntegrityChecker sic;

    bool loop = true;
    bool rs = true;
    bool fs = false;
    bool bs = false;

    m::Window win;
    testAssert(win.create("MGPCL TEST", 800, 450), "coudln't create window");
    win.show();

#ifdef MGPCL_WIN
    m::FunctionalThread thread([&win] ()
    {
        m::time::sleepMs(400);
        SetActiveWindow(win.win32Wnd());
        m::time::sleepMs(100);
        PostMessage(win.win32Wnd(), WM_KEYUP, static_cast<WPARAM>(m::kKC_R), 0);
        m::time::sleepMs(100);
        PostMessage(win.win32Wnd(), WM_KEYUP, static_cast<WPARAM>(m::kKC_R), 0);
        m::time::sleepMs(100);
        PostMessage(win.win32Wnd(), WM_KEYUP, static_cast<WPARAM>(m::kKC_I), 0);
        m::time::sleepMs(100);
        PostMessage(win.win32Wnd(), WM_KEYUP, static_cast<WPARAM>(m::kKC_Enter), 0);
        m::time::sleepMs(100);
        PostMessage(win.win32Wnd(), WM_KEYUP, static_cast<WPARAM>(m::kKC_F), 0);
        m::time::sleepMs(250);
        PostMessage(win.win32Wnd(), WM_KEYUP, static_cast<WPARAM>(m::kKC_F), 0);
        m::time::sleepMs(250);
        PostMessage(win.win32Wnd(), WM_KEYUP, static_cast<WPARAM>(m::kKC_B), 0);
        m::time::sleepMs(100);
        PostMessage(win.win32Wnd(), WM_KEYUP, static_cast<WPARAM>(m::kKC_B), 0);
        m::time::sleepMs(100);
        PostMessage(win.win32Wnd(), WM_KEYUP, static_cast<WPARAM>(m::kKC_Escape), 0);
    });

    thread.start();
#endif

    while(loop) {
        m::WindowEvent &ev = win.lastEvent();
        win.waitEvent();

        switch(ev.type) {
        case m::kWET_Closing:
            loop = false;
            break;

        case m::kWET_KeyDown:
            std::cout << "[i]\tKey down KC=\"" << m::keyboard::keycodeLabel(ev.keycode) << "\", SC=\"" << m::keyboard::scancodeLabel(ev.scancode) << "\", rep=" << ev.isRepeat << std::endl;
            break;

        case m::kWET_KeyUp:
            std::cout << "[i]\tKey up KC=\"" << m::keyboard::keycodeLabel(ev.keycode) << "\", SC=\"" << m::keyboard::scancodeLabel(ev.scancode) << '\"' << std::endl;
            if(ev.keycode == m::kKC_Escape)
                loop = false;

            if(ev.keycode == m::kKC_R) {
                rs = !rs;
                win.setResizable(rs);
            }

            if(ev.keycode == m::kKC_I)
                win.setIcon(g_iconData.pixel_data, g_iconData.width, g_iconData.height);

            if(ev.keycode == m::kKC_Enter)
                win.resize(640, 360);

            if(ev.keycode == m::kKC_F) {
                fs = !fs;
                win.setFullscreen(fs);
            }

            if(ev.keycode == m::kKC_B) {
                bs = !bs;
                win.setBorderless(bs);
            }

            break;

        case m::kWET_KeyType:
            if(ev.text[0] != 0) {
                if(ev.text[0] == '\r' || ev.text[0] == '\n' || ev.text[0] == '\t' || ev.text[0] == '\b') {
                    char tmp[3] = { '\\', unescape(ev.text[0]), 0 };
                    std::cout << "[i]\tWrote \'" << tmp << '\'' << std::endl;
                } else
                    std::cout << "[i]\tWrote \'" << ev.text << '\'' << std::endl;
            }

            break;

        default:
            break;
        }
    }

#ifdef MGPCL_WIN
    thread.join();
#endif

    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;
    m::List<m::WindowRect> screens;
    m::Window::getMonitorsGeometry(screens);

    testAssert(screens.size() >= 1, "invalid screen sizes");
    for(m::WindowRect &wr: screens)
        std::cout << "[i]\tMonitor @" << wr.x << "," << wr.y << " of size [" << wr.w << "," << wr.h << "]" << std::endl;

    return true;
}

#endif
