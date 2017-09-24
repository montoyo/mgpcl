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
#include "Config.h"

#ifdef M_KEYBOARD_DECLARE
#define M_KEYBOARD_PREFIX
#else
#define M_KEYBOARD_PREFIX extern
#endif

namespace m
{
    enum ScanCode
    {
        kSC_Escape = 1,
        kSC_1 = 2,
        kSC_ExclamationMark = 2,
        kSC_2 = 3,
        kSC_At = 3, //@ key
        kSC_3 = 4,
        kSC_Sharp = 4, //# key
        kSC_4 = 5,
        kSC_Dollar = 5, //$ key
        kSC_5 = 6,
        kSC_Percent = 6, //% key
        kSC_6 = 7,
        kSC_InvertedBreve = 7, //^ key
        kSC_7 = 8,
        kSC_And = 8, //& key
        kSC_8 = 9,
        kSC_Star = 9, //* key
        kSC_9 = 10,
        kSC_OpenParenthesis = 10, //( key
        kSC_0 = 11,
        kSC_CloseParenthesis = 11, //) key
        kSC_Minus = 12, //- key
        kSC_Underscore = 12, //_ key
        kSC_Equals = 13, //= key
        kSC_Plus = 13, //+ key
        kSC_Backspace = 14,
        kSC_Tab = 15,
        kSC_Q = 16,
        kSC_W = 17,
        kSC_E = 18,
        kSC_R = 19,
        kSC_T = 20,
        kSC_Y = 21,
        kSC_U = 22,
        kSC_I = 23,
        kSC_O = 24,
        kSC_P = 25,
        kSC_OpenBracket = 26, //[ key
        kSC_OpenBrace = 26, //{ key
        kSC_CloseBracket = 27, //] key
        kSC_CloseBrace = 27, //} key
        kSC_Enter = 28,
        kSC_Ctrl = 29,
        kSC_A = 30,
        kSC_S = 31,
        kSC_D = 32,
        kSC_F = 33,
        kSC_G = 34,
        kSC_H = 35,
        kSC_J = 36,
        kSC_K = 37,
        kSC_L = 38,
        kSC_Semicolon = 39, //; key
        kSC_Colons = 39, //: key
        kSC_SingleQuote = 40, //' key
        kSC_Quote = 40, //" key
        kSC_Grave = 41, //` key
        kSC_Tilde = 41, //~ key
        kSC_LShift = 42,
        kSC_Backslash = 43, //\ key
        kSC_Pipe = 43, //| key
        kSC_Z = 44,
        kSC_X = 45,
        kSC_C = 46,
        kSC_V = 47,
        kSC_B = 48,
        kSC_N = 49,
        kSC_M = 50,
        kSC_Comma = 51, //, key
        kSC_LessThan = 51, //< key
        kSC_Period = 52, //. key
        kSC_GreaterThan = 52, //> key
        kSC_Slash = 53, // / key
        kSC_QuestionMark = 53, //? key
        kSC_RShift = 54,
        kSC_PrintScr = 55,
        kSC_Alt = 56,
        kSC_Space = 57,
        kSC_CapsLock = 58,
        kSC_F1 = 59,
        kSC_F2 = 60,
        kSC_F3 = 61,
        kSC_F4 = 62,
        kSC_F5 = 63,
        kSC_F6 = 64,
        kSC_F7 = 65,
        kSC_F8 = 66,
        kSC_F9 = 67,
        kSC_F10 = 68,
        kSC_F11 = 87,
        kSC_F12 = 88,
        kSC_NumLock = 69,
        kSC_ScrollLock = 70,
        kSC_Home = 71,
        kSC_Up = 72,
        kSC_PageUp = 73,
        kSC_Left = 75,
        kSC_Right = 77,
        kSC_End = 79,
        kSC_Down = 80,
        kSC_PageDown = 81,
        kSC_Insert = 82,
        kSC_Delete = 83,

        kSC_KP0 = 260,
        kSC_KP1 = 261,
        kSC_KP2 = 262,
        kSC_KP3 = 263,
        kSC_KP4 = 264,
        kSC_KP5 = 265,
        kSC_KP6 = 266,
        kSC_KP7 = 267,
        kSC_KP8 = 268,
        kSC_KP9 = 269,
        kSC_KPSlash = 270,
        kSC_KPStar = 271,
        kSC_KPMinus = 272,
        kSC_KPPlus = 273,
        kSC_KPEnter = 274,
        kSC_KPPeriod = 275,

        kSC_Max //Not a scancode!
    };

    enum KeyCode
    {
        kKC_Backspace = 0x08,
        kKC_Tab = 0x09,
        kKC_Clear = 0x0C,
        kKC_Enter = 0x0D,
        kKC_Shift = 0x10,
        kKC_Control = 0x11,
        kKC_Alt = 0x12,
        kKC_Pause = 0x13,
        kKC_CapsLock = 0x14,
        kKC_Escape = 0x1B,
        kKC_Space = 0x20,
        kKC_PageUp = 0x21,
        kKC_PageDown = 0x22,
        kKC_End = 0x23,
        kKC_Home = 0x24,
        kKC_Left = 0x25,
        kKC_Up = 0x26,
        kKC_Right = 0x27,
        kKC_Down = 0x28,
        kKC_Select = 0x29,
        kKC_Print = 0x2A,
        kKC_Execute = 0x2B,
        kKC_PrintScr = 0x2C,
        kKC_Insert = 0x2D,
        kKC_Delete = 0x2E,
        kKC_Help = 0x2F,
        kKC_0 = 0x30,
        kKC_1 = 0x31,
        kKC_2 = 0x32,
        kKC_3 = 0x33,
        kKC_4 = 0x34,
        kKC_5 = 0x35,
        kKC_6 = 0x36,
        kKC_7 = 0x37,
        kKC_8 = 0x38,
        kKC_9 = 0x39,
        kKC_A = 0x41,
        kKC_B = 0x42,
        kKC_C = 0x43,
        kKC_D = 0x44,
        kKC_E = 0x45,
        kKC_F = 0x46,
        kKC_G = 0x47,
        kKC_H = 0x48,
        kKC_I = 0x49,
        kKC_J = 0x4A,
        kKC_K = 0x4B,
        kKC_L = 0x4C,
        kKC_M = 0x4D,
        kKC_N = 0x4E,
        kKC_O = 0x4F,
        kKC_P = 0x50,
        kKC_Q = 0x51,
        kKC_R = 0x52,
        kKC_S = 0x53,
        kKC_T = 0x54,
        kKC_U = 0x55,
        kKC_V = 0x56,
        kKC_W = 0x57,
        kKC_X = 0x58,
        kKC_Y = 0x59,
        kKC_Z = 0x5A,
        kKC_LSuper = 0x5B, //Left Windows flag key
        kKC_RSuper = 0x5C, //Right Windows flag key
        kKC_Application = 0x5D,
        kKC_Sleep = 0x5F,
        kKC_KP0 = 0x60,
        kKC_KP1 = 0x61,
        kKC_KP2 = 0x62,
        kKC_KP3 = 0x63,
        kKC_KP4 = 0x64,
        kKC_KP5 = 0x65,
        kKC_KP6 = 0x66,
        kKC_KP7 = 0x67,
        kKC_KP8 = 0x68,
        kKC_KP9 = 0x69,
        kKC_KPStar = 0x6A,
        kKC_KPPlus = 0x6B,
        kKC_KPSeparator = 0x6C,
        kKC_KPMinus = 0x6D,
        kKC_KPPeriod = 0x6E,
        kKC_KPSlash = 0x6F,
        kKC_F1 = 0x70,
        kKC_F2 = 0x71,
        kKC_F3 = 0x72,
        kKC_F4 = 0x73,
        kKC_F5 = 0x74,
        kKC_F6 = 0x75,
        kKC_F7 = 0x76,
        kKC_F8 = 0x77,
        kKC_F9 = 0x78,
        kKC_F10 = 0x79,
        kKC_F11 = 0x80,
        kKC_F12 = 0x81,
        kKC_F13 = 0x7C,
        kKC_F14 = 0x7D,
        kKC_F15 = 0x7E,
        kKC_F16 = 0x7F,
        kKC_F17 = 0x80,
        kKC_F18 = 0x81,
        kKC_F19 = 0x82,
        kKC_F20 = 0x83,
        kKC_F21 = 0x84,
        kKC_F22 = 0x85,
        kKC_F23 = 0x86,
        kKC_F24 = 0x87,
        kKC_NumLock = 0x90,
        kKC_ScrollLock = 0x91,
        kKC_LShift = 0xA0,
        kKC_RShift = 0xA1,
        kKC_LControl = 0xA2,
        kKC_RControl = 0xA3,
        kKC_LAlt = 0xA4,
        kKC_RAlt = 0xA5,
        kKC_SemiColon = 0xBA, //US layout only!
        kKC_Colons = 0xBA, //US layout only!!
        kKC_Plus = 0xBB,
        kKC_Comma = 0xBC,
        kKC_Minus = 0xBD,
        kKC_Period = 0xBE,
        kKC_Slash = 0xBF, //US layout only!!
        kKC_QuestionMark = 0xBF, //US layout only!!
        kKC_Grave = 0xC0, //US layout only!!
        kKC_Tilde = 0xC0, //US layout only!!
        kKC_OpenBracket = 0xDB, //US layout only!!
        kKC_OpenBrace = 0xDB, //US layout only!!
        kKC_Backslash = 0xDC, //US layout only!!
        kKC_Pipe = 0xDC, //US layout only!!
        kKC_CloseBracket = 0xDD, //US layout only!!
        kKC_CloseBrace = 0xDD, //US layout only!!
        kKC_SingleQuote = 0xDE, //US layout only!!
        kKC_Quote = 0xDE, //US layout only!!

        kKC_KPEnter = 0x110,
        kKC_Max //Not a keycode!
    };

    namespace keyboard
    {
        M_KEYBOARD_PREFIX const char *keycodeName(KeyCode kc);
        M_KEYBOARD_PREFIX const char *keycodeLabel(KeyCode kc);
        M_KEYBOARD_PREFIX const char *scancodeName(ScanCode kc);
        M_KEYBOARD_PREFIX const char *scancodeLabel(ScanCode kc);

#ifdef MGPCL_LINUX
        M_KEYBOARD_PREFIX KeyCode remapKeycode(unsigned int src);
        M_KEYBOARD_PREFIX ScanCode remapScancode(unsigned short src);
#endif
    }

}
