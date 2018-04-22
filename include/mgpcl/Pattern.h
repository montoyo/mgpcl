/* Copyright (C) 2018 BARBOTIN Nicolas
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
        kPPE_MisplacedCtrlChar
    };

    class Matcher
    {
    public:
    	//Matcher();
    	//~Matcher();

    private:
        String *m_groups;
    };

    class PatternNode;

    class Pattern
    {
    public:
        Pattern();
        ~Pattern();

        bool compile(const String &str);
        Matcher *matcher(const String &str);

        PatternParseError parseError() const
        {
            return m_err;
        }

        const char *parseErrorString() const;

    private:
        PatternNode *m_root;
        uint32_t m_flags;
        PatternParseError m_err;
    };
}
