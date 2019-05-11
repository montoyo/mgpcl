/* Copyright (C) 2019 BARBOTIN Nicolas
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

#ifdef MGPCL_ENABLE_PATTERNS

namespace m
{
    enum PatternParseError
    {
        kPPE_NoError = 0,
        kPPE_UnclosedParenthesis, //'(' without ')'
        kPPE_UnclosedBracket,     //'[' without ']'
        kPPE_InvalidRange,        //i.e. [Z-A] instead of [A-Z]
        kPPE_MisplacedEscape,     //'%' at the end of a range, capture, or pattern
        kPPE_EmptyCapture,        //"()" found in pattern
        kPPE_MisplacedCtrlChar,
        kPPE_EmptyPattern
    };

    class Pattern;

    class Matcher
    {
        friend class Pattern;

    public:
        bool next();

        String capture(int i = 0) const
        {
            return m_str.substr(m_starts[i], m_ends[i]);
        }

        int captureBegin(int i = 0) const
        {
            return m_starts[i];
        }

        int captureEnd(int i = 0) const
        {
            return m_ends[i];
        }

        int numCaptures() const
        {
            return ~m_starts;
        }

        const String &originalString() const
        {
            return m_str;
        }

    private:
        Matcher()
        {
        }

        Matcher(Pattern *p, const String &str) : m_pat(p), m_str(str), m_strPos(0)
        {
        }

        Matcher(Pattern *p, String &&str) : m_pat(p), m_str(str), m_strPos(0)
        {
        }

        int matches();

        Pattern *m_pat;
        String m_str;
        int m_strPos;
        List<int> m_starts;
        List<int> m_ends;
    };

    class PatternNode;

    class Pattern
    {
        friend class Matcher;

    public:
        Pattern() : m_root(nullptr), m_flags(0), m_err(kPPE_NoError)
        {
        }

        explicit Pattern(const String &str) : m_root(nullptr), m_flags(0), m_err(kPPE_NoError)
        {
            compile(str.raw(), str.length());
        }

        explicit Pattern(const char *str, int len = -1) : m_root(nullptr), m_flags(0), m_err(kPPE_NoError)
        {
            compile(str, len);
        }

        ~Pattern();
        bool compile(const char *sIt, int sLen = -1);
        const char *parseErrorString() const;
        
        Matcher matcher(const String &str)
        {
            mAssert(m_root != nullptr, "can't make matcher out of invalid pattern");
            return Matcher(this, str);
        }

        Matcher matcher(String &&str)
        {
            mAssert(m_root != nullptr, "can't make matcher out of invalid pattern");
            return Matcher(this, str);
        }

        Matcher matcher(const char *str, int len = -1)
        {
            mAssert(m_root != nullptr, "can't make matcher out of invalid pattern");
            return Matcher(this, String(str, len));
        }

        bool operator == (const String &str)
        {
            mAssert(m_root != nullptr, "can't make matcher out of invalid pattern");
            return Matcher(this, str).next();
        }

        bool operator == (String &&str)
        {
            mAssert(m_root != nullptr, "can't make matcher out of invalid pattern");
            return Matcher(this, str).next();
        }

        bool operator == (const char *str)
        {
            mAssert(m_root != nullptr, "can't make matcher out of invalid pattern");
            return Matcher(this, String(str)).next();
        }

        bool operator != (const String &str)
        {
            mAssert(m_root != nullptr, "can't make matcher out of invalid pattern");
            return !Matcher(this, str).next();
        }

        bool operator != (String &&str)
        {
            mAssert(m_root != nullptr, "can't make matcher out of invalid pattern");
            return !Matcher(this, str).next();
        }

        bool operator != (const char *str)
        {
            mAssert(m_root != nullptr, "can't make matcher out of invalid pattern");
            return !Matcher(this, String(str)).next();
        }

        bool compile(const String &str)
        {
            return compile(str.raw(), str.length());
        }

        PatternParseError parseError() const
        {
            return m_err;
        }

        bool operator ! () const
        {
            return m_root == nullptr;
        }

        bool isValid() const
        {
            return m_root != nullptr;
        }

    private:
        PatternNode *m_root;
        uint32_t m_flags;
        PatternParseError m_err;
    };
}

#endif
