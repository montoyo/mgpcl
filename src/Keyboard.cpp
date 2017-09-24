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

#define M_KEYBOARD_DECLARE
#include "mgpcl/Keyboard.h"

#define M_UNKNOWN_KEY "Unknown"
const char *g_keycodeNames[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Backspace", "Tab", nullptr, nullptr, "Clear", "Enter", nullptr, nullptr, "Shift", "Control", "Alt", "Pause", "CapsLock", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Escape", nullptr, nullptr, nullptr, nullptr, "Space", "PageUp", "PageDown", "End", "Home", "Left", "Up", "Right", "Down", "Select", "Print", "Execute", "PrintScr", "Insert", "Delete", "Help", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "LSuper", "RSuper", "Application", nullptr, "Sleep", "KP0", "KP1", "KP2", "KP3", "KP4", "KP5", "KP6", "KP7", "KP8", "KP9", "KPStar", "KPPlus", "KPSeparator", "KPMinus", "KPPeriod", "KPSlash", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", nullptr, nullptr, "F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "NumLock", "ScrollLock", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "LShift", "RShift", "LControl", "RControl", "LAlt", "RAlt", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Colons", "Plus", "Comma", "Minus", "Period", "QuestionMark", "Tilde", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "OpenBrace", "Pipe", "CloseBrace", "Quote", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "KPEnter" };
const char *g_keycodeLabels[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Backspace", "Tab", nullptr, nullptr, "Clear", "Enter", nullptr, nullptr, "Shift", "Control", "Alt", "Pause", "Caps Lock", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Escape", nullptr, nullptr, nullptr, nullptr, "Space", "Page Up", "Page Down", "End", "Home", "Left", "Up", "Right", "Down", "Select", "Print", "Execute", "Print Screen", "Insert", "Delete", "Help", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Left Super", "Right Super", "Application", nullptr, "Sleep", "Keypad 0", "Keypad 1", "Keypad 2", "Keypad 3", "Keypad 4", "Keypad 5", "Keypad 6", "Keypad 7", "Keypad 8", "Keypad 9", "Keypad Multiply", "Keypad Add", "Keypad Separator", "Keypad Subtract", "Keypad Decimal", "Keypad Divide", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", nullptr, nullptr, "F13", "F14", "F15", "F16", "F11", "F12", "F19", "F20", "F21", "F22", "F23", "F24", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Num Lock", "Scroll Lock", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Left Shift", "Right Shift", "Left Control", "Right Control", "Left Alt", "Right Alt", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Semicolon", "Plus", "Comma", "Minus", "Period", "Slash", "Grave", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Open Bracket", "Backslash", "Close Bracket", "Single Quote", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Keypad Enter" };
const char *g_scancodeNames[] = { nullptr, "Escape", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Minus", "Equals", "Backspace", "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "OpenBracket", "CloseBracket", "Enter", "Ctrl", "A", "S", "D", "F", "G", "H", "J", "K", "L", "Semicolon", "SingleQuote", "Grave", "LShift", "Backslash", "Z", "X", "C", "V", "B", "N", "M", "Comma", "Period", "Slash", "RShift", "PrintScr", "Alt", "Space", "CapsLock", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NumLock", "ScrollLock", "Home", "Up", "PageUp", nullptr, "Left", nullptr, "Right", nullptr, "End", "Down", "PageDown", "Insert", "Delete", nullptr, nullptr, nullptr, "F11", "F12", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "KP0", "KP1", "KP2", "KP3", "KP4", "KP5", "KP6", "KP7", "KP8", "KP9", "KPSlash", "KPStar", "KPMinus", "KPPlus", "KPEnter", "KPPeriod" };
const char *g_scancodeLabels[] = { nullptr, "Escape", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Minus", "Equals", "Backspace", "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "Open Bracket", "Close Bracket", "Enter", "Ctrl", "A", "S", "D", "F", "G", "H", "J", "K", "L", "Semicolon", "Single Quote", "Grave", "Left Shift", "Backslash", "Z", "X", "C", "V", "B", "N", "M", "Comma", "Period", "Slash", "Right Shift", "Print Screen", "Alt", "Space", "Caps Lock", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "Num Lock", "Scroll Lock", "Home", "Up", "Page Up", nullptr, "Left", nullptr, "Right", nullptr, "End", "Down", "Page Down", "Insert", "Delete", nullptr, nullptr, nullptr, "F11", "F12", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Keypad 0", "Keypad 1", "Keypad 2", "Keypad 3", "Keypad 4", "Keypad 5", "Keypad 6", "Keypad 7", "Keypad 8", "Keypad 9", "Keypad Divide", "Keypad Multiply", "Keypad Subtract", "Keypad Add", "Keypad Enter", "Keypad Decimal" };

const char *m::keyboard::keycodeName(KeyCode kc)
{
    if(kc >= kKC_Max)
        return M_UNKNOWN_KEY;

    const char *ret = g_keycodeNames[kc];
    if(ret == nullptr)
        ret = M_UNKNOWN_KEY;

    return ret;
}

const char *m::keyboard::keycodeLabel(KeyCode kc)
{
    if(kc >= kKC_Max)
        return M_UNKNOWN_KEY;

    const char *ret = g_keycodeLabels[kc];
    if(ret == nullptr)
        ret = M_UNKNOWN_KEY;

    return ret;
}

const char *m::keyboard::scancodeName(ScanCode sc)
{
    if(sc >= kSC_Max)
        return M_UNKNOWN_KEY;

    const char *ret = g_scancodeNames[sc];
    if(ret == nullptr)
        ret = M_UNKNOWN_KEY;

    return ret;
}

const char *m::keyboard::scancodeLabel(ScanCode sc)
{
    if(sc >= kSC_Max)
        return M_UNKNOWN_KEY;

    const char *ret = g_scancodeLabels[sc];
    if(ret == nullptr)
        ret = M_UNKNOWN_KEY;

    return ret;
}

#ifdef MGPCL_LINUX
namespace m
{
    static KeyCode g_kcRemap00[256] = { kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Space, kKC_Max, kKC_Quote, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Plus, kKC_Comma, kKC_Minus, kKC_Period, kKC_QuestionMark, kKC_0, kKC_1, kKC_2, kKC_3, kKC_4, kKC_5, kKC_6, kKC_7, kKC_8, kKC_9, kKC_Colons, kKC_Colons, kKC_Max, kKC_Max, kKC_Max, kKC_QuestionMark, kKC_Max, kKC_A, kKC_B, kKC_C, kKC_D, kKC_E, kKC_F, kKC_G, kKC_H, kKC_I, kKC_J, kKC_K, kKC_L, kKC_M, kKC_N, kKC_O, kKC_P, kKC_Q, kKC_R, kKC_S, kKC_T, kKC_U, kKC_V, kKC_W, kKC_X, kKC_Y, kKC_Z, kKC_OpenBrace, kKC_Pipe, kKC_CloseBrace, kKC_Max, kKC_Max, kKC_Quote, kKC_A, kKC_B, kKC_C, kKC_D, kKC_E, kKC_F, kKC_G, kKC_H, kKC_I, kKC_J, kKC_K, kKC_L, kKC_M, kKC_N, kKC_O, kKC_P, kKC_Q, kKC_R, kKC_S, kKC_T, kKC_U, kKC_V, kKC_W, kKC_X, kKC_Y, kKC_Z, kKC_OpenBrace, kKC_Pipe, kKC_CloseBrace, kKC_Tilde, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max };
    static KeyCode g_kcRemapFF[256] = { kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Backspace, kKC_Tab, kKC_Max, kKC_Clear, kKC_Max, kKC_Enter, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Pause, kKC_ScrollLock, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Escape, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Home, kKC_Left, kKC_Up, kKC_Right, kKC_Down, kKC_PageUp, kKC_PageDown, kKC_End, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Select, kKC_Print, kKC_Execute, kKC_Insert, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Help, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_NumLock, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_KPEnter, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_KPStar, kKC_KPPlus, kKC_KPSeparator, kKC_KPMinus, kKC_KPPeriod, kKC_KPSlash, kKC_KP0, kKC_KP1, kKC_KP2, kKC_KP3, kKC_KP4, kKC_KP5, kKC_KP6, kKC_KP7, kKC_KP8, kKC_KP9, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_F1, kKC_F2, kKC_F3, kKC_F4, kKC_F5, kKC_F6, kKC_F7, kKC_F8, kKC_F9, kKC_F10, kKC_F17, kKC_F18, kKC_F13, kKC_F14, kKC_F15, kKC_F16, kKC_F17, kKC_F18, kKC_F19, kKC_F20, kKC_F21, kKC_F22, kKC_F23, kKC_F24, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_LShift, kKC_RShift, kKC_LControl, kKC_RControl, kKC_CapsLock, kKC_Max, kKC_Max, kKC_Max, kKC_LAlt, kKC_RAlt, kKC_LSuper, kKC_RSuper, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Max, kKC_Delete };
}

m::KeyCode m::keyboard::remapKeycode(unsigned int src)
{
    if(src == 0xFD1D)
        return kKC_PrintScr;

    if((src & 0xFF00) == 0x0000)
        return g_kcRemap00[src & 0x00FF];

    if((src & 0xFF00) == 0xFF00)
        return g_kcRemapFF[src & 0x00FF];

    return kKC_Max;
}

m::ScanCode m::keyboard::remapScancode(unsigned short src)
{
    src -= 8;
    if(src == 103)
        return kSC_Up;
    else if(src == 105)
        return kSC_Left;
    else if(src == 106)
        return kSC_Right;
    else if(src == 108)
        return kSC_Down;
    else if(src == 111)
        return kSC_Delete;

    if(src >= kSC_Max)
        return kSC_Max;

    return static_cast<ScanCode>(src);
}
#endif

