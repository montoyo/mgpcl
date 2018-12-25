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

#define M_CUTILS_SRC
#include "mgpcl/ConsoleUtils.h"

#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static volatile LONG g_attrsSet = 0;
static volatile WORD g_attrs = 0;

static inline void backupAttributes(WORD def)
{
    if(InterlockedCompareExchange(&g_attrsSet, 1, 0) == 0)
        g_attrs = def;
}
#else
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

static int toLinuxColor(m::ConsoleColor cc)
{
    //Could use a mapping, really...
    switch(cc) {
    case m::kCC_Black:
        return 0;

    case m::kCC_Blue:
    case m::kCC_WeirdBlue:
    case m::kCC_LightBlue:
        return 4;

    case m::kCC_Green:
    case m::kCC_WeirdGreen:
    case m::kCC_LightGreen:
        return 2;

    case m::kCC_Red:
    case m::kCC_Pink:
        return 1;

    case m::kCC_WeirdPurple:
        return 5;

    case m::kCC_Cyan:
        return 6;

    case m::kCC_WeirdBrown:
    case m::kCC_Yellow:
        return 3;

    case m::kCC_LightGray:
    case m::kCC_Gray:
    case m::kCC_White:
        return 7;

    default:
        return 0;
    }
}
#endif

void m::console::setTitle(const String &title)
{
#ifdef MGPCL_WIN
    SetConsoleTitle(title.raw());
#else
    std::cout << "\033]0;" << title.raw() << '\007';
#endif
}

void m::console::setTextColor(ConsoleColor cc)
{
#ifdef MGPCL_WIN
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(out, &csbi);
    backupAttributes(csbi.wAttributes);

    WORD attr = csbi.wAttributes & 0xFFF0;
    attr |= static_cast<WORD>(cc);

    SetConsoleTextAttribute(out, attr);
#else
    std::cout << "\033[" << 30 + toLinuxColor(cc) << 'm';
#endif
}

void m::console::setBackgroundColor(ConsoleColor cc)
{
#ifdef MGPCL_WIN
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(out, &csbi);
    backupAttributes(csbi.wAttributes);

    WORD attr = csbi.wAttributes & 0xFF0F;
    attr |= static_cast<WORD>(cc) << 4;

    SetConsoleTextAttribute(out, attr);
#else
    std::cout << "\033[" << 40 + toLinuxColor(cc) << 'm';
#endif
}

void m::console::setColor(ConsoleColor fg, ConsoleColor bg)
{
#ifdef MGPCL_WIN
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(out, &csbi);
    backupAttributes(csbi.wAttributes);

    WORD attr = csbi.wAttributes & 0xFF00;
    attr |= static_cast<WORD>(fg) << 0;
    attr |= static_cast<WORD>(bg) << 4;

    SetConsoleTextAttribute(out, attr);
#else
    std::cout << "\033[" << 30 + toLinuxColor(fg) << ';' << 40 + toLinuxColor(bg) << 'm';
#endif
}

m::Vector2i m::console::getSize()
{
#ifdef MGPCL_WIN
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

    return Vector2i(static_cast<int>(csbi.dwSize.X), static_cast<int>(csbi.dwSize.Y));
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return Vector2i(static_cast<int>(w.ws_col), static_cast<int>(w.ws_row));
#endif
}

m::Vector2i m::console::getCursorPos()
{
#ifdef MGPCL_WIN
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

    return Vector2i(static_cast<int>(csbi.dwCursorPosition.X), static_cast<int>(csbi.dwCursorPosition.Y));
#else
    std::cout << "\033[6n";

    struct termios backup, empty;
    tcgetattr(STDIN_FILENO, &backup);
    cfmakeraw(&empty);
    tcsetattr(STDIN_FILENO, TCSANOW, &empty);

    char chr1 = 0;
    char chr2 = 0;
    while(chr1 != '\033' || chr2 != '[')
        std::cin >> chr1 >> chr2;

    int line, col;
    std::cin >> line >> chr1 >> col >> chr2; //That should do it; nah?

    tcsetattr(STDIN_FILENO, TCSANOW, &backup);
    return Vector2i(col - 1, line - 1);
#endif
}

void m::console::setCursorPos(int x, int y)
{
#ifdef MGPCL_WIN
    COORD c;
    c.X = static_cast<SHORT>(x);
    c.Y = static_cast<SHORT>(y);

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
#else
    std::cout << "\033[" << y + 1 << ';' << x + 1 << 'H';
#endif
}

void m::console::clearLastLine()
{
#ifdef MGPCL_WIN
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD lineBegin;

    GetConsoleScreenBufferInfo(out, &csbi);
    lineBegin.X = 0;
    lineBegin.Y = csbi.dwCursorPosition.Y;

    DWORD noCare;
    FillConsoleOutputCharacter(out, ' ', static_cast<DWORD>(csbi.dwSize.X), lineBegin, &noCare);
    FillConsoleOutputAttribute(out, csbi.wAttributes, csbi.dwSize.X, lineBegin, &noCare);
    SetConsoleCursorPosition(out, lineBegin);
#else
    std::cout << "\033[K";
#endif
}

void m::console::clear()
{
#ifdef MGPCL_WIN
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD begin = { 0, 0 };

    GetConsoleScreenBufferInfo(out, &csbi);

    DWORD sz = static_cast<DWORD>(csbi.dwSize.X) * static_cast<DWORD>(csbi.dwSize.Y);
    DWORD noCare;

    FillConsoleOutputCharacter(out, ' ', sz, begin, &noCare);
    FillConsoleOutputAttribute(out, csbi.wAttributes, sz, begin, &noCare);
    SetConsoleCursorPosition(out, begin);
#else
    std::cout << "\033[2J";
#endif
}

void m::console::resetColor()
{
#ifdef MGPCL_WIN
    volatile LONG status;

    do {
        status = g_attrsSet;
    } while(InterlockedCompareExchange(&g_attrsSet, status, status) != status);

    if(status != 0)
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), g_attrs);
#else
    std::cout << "\033[0m";
#endif
}

static m::ConsoleCtrlCHandler *g_ctrlCHandler = nullptr;

#ifdef MGPCL_WIN
static BOOL WINAPI g_consoleHandlerFunc(DWORD sig)
{
    if(sig == CTRL_C_EVENT && g_ctrlCHandler != nullptr)
        g_ctrlCHandler->handleCtrlC();

    return TRUE;
}
#else
static void g_consoleHandlerFunc(int sig)
{
    if(g_ctrlCHandler != nullptr)
        g_ctrlCHandler->handleCtrlC();
}
#endif

void m::console::setCtrlCHandler(ConsoleCtrlCHandler *h)
{
    if(h != g_ctrlCHandler) {
        if(g_ctrlCHandler != nullptr)
            delete g_ctrlCHandler;

        g_ctrlCHandler = h;

#ifdef MGPCL_WIN
        if(h == nullptr)
            SetConsoleCtrlHandler(nullptr, FALSE);
        else
            SetConsoleCtrlHandler(g_consoleHandlerFunc, TRUE);
#else
        if(h == nullptr)
            signal(SIGINT, SIG_DFL);
        else
            signal(SIGINT, g_consoleHandlerFunc);
#endif
    }
}

class FuncCtrlCHandler : public m::ConsoleCtrlCHandler
{
public:
    FuncCtrlCHandler(std::function<void()> func) : m_func(func) {}

    void handleCtrlC() override
    {
        m_func();
    }

private:
    std::function<void()> m_func;
};

void m::console::setCtrlCHandler(std::function<void()> func)
{
    setCtrlCHandler(new FuncCtrlCHandler(func));
}
