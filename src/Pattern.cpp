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

#include "mgpcl/Pattern.h"
//#define M_PAT_TEST_MEMLEAK

#ifdef M_PAT_TEST_MEMLEAK
static int g_nodeCount = 0;
#endif

namespace m
{
    typedef const char *CStringIterator;

    enum SpecialNodeType
    {
        kSNT_None = 0,
        kSNT_Char,
        kSNT_End
    };

    enum PatternFlag
    {
        kPF_Start = 1,
        kPF_End = 2
    };

    enum NodeMark
    {
        kNM_OptimizePass = 1,
        kNM_DFS = 2
    };

    class PatternNode
    {
    public:
        PatternNode() : m_marks(0), m_invert(false)
        {
#ifdef M_PAT_TEST_MEMLEAK
            g_nodeCount++;
#endif
        }

        virtual ~PatternNode()
        {
#ifdef M_PAT_TEST_MEMLEAK
            g_nodeCount--;
#endif
        }

        virtual bool matches(CStringIterator &it, int &len) = 0;
        void optimize();

        int numChildren() const
        {
            return ~m_next;
        }

        PatternNode *child(int id)
        {
            return m_next[id];
        }

        void addChild(PatternNode *c)
        {
            m_next.add(c);
        }

        void dfs(List<PatternNode*> &dst)
        {
            dst.add(this);
            m_marks |= kNM_DFS;

            for(PatternNode *pn: m_next) {
                if((pn->m_marks & kNM_DFS) == 0)
                    pn->dfs(dst);
            }
        }

        //Special types handling
        virtual SpecialNodeType specialType() const
        {
            return kSNT_None;
        }

        void invert()
        {
            m_invert = true;
        }

        bool isInverted() const
        {
            return m_invert;
        }

    private:
        List<PatternNode*> m_next;
        uint8_t m_marks;

    protected:
        bool m_invert;
    };

    class PatternNodeChar : public PatternNode
    {
    public:
        PatternNodeChar() = default;
        PatternNodeChar(char chr) : m_char(chr) {}

        virtual bool matches(CStringIterator &it, int &len) override
        {
            return --len >= 0 && (*(it++) == m_char) != m_invert;
        }

        virtual SpecialNodeType specialType() const override
        {
            return kSNT_Char;
        }

        char chr() const
        {
            return m_char;
        }

    private:
        char m_char;
    };

    /*class PatternNodeCharSequence : public PatternNode
    {
    public:
        PatternNodeCharSequence() = default;
        PatternNodeCharSequence(m::String &&str) : m_charSeq(str) {}

        virtual bool matches(CStringIterator &it, int &len) override
        {
            if(len >= m_charSeq.length() && m_charSeq.equals(it, m_charSeq.length())) {
                len -= m_charSeq.length();
                it += m_charSeq.length();

                return true;
            } else
                return false;
        }

    private:
        String m_charSeq;
    };*/

    class PatternNodeCharRange : public PatternNode
    {
    public:
        PatternNodeCharRange() = default;
        PatternNodeCharRange(char cmin, char cmax) : m_min(cmin), m_max(cmax) {}

        virtual bool matches(CStringIterator &it, int &len) override
        {
            return --len >= 0 && (*it >= m_min && *(it++) <= m_max) != m_invert;
        }

    private:
        char m_min;
        char m_max;
    };

    class PatternNodeMultiRange : public PatternNode
    {
    public:
        class Bounds
        {
        public:
            Bounds() : cmin(0), cmax(0) {}
            Bounds(char chr) : cmin(chr), cmax(chr) {}
            Bounds(char c1, char c2) : cmin(c1), cmax(c2) {}

            char cmin;
            char cmax;
        };

        PatternNodeMultiRange() = default;
        PatternNodeMultiRange(m::List<Bounds> &&lst) : m_bounds(lst) {}

        virtual bool matches(CStringIterator &it, int &len) override
        {
            if(--len >= 0) {
                char chr = *(it++);

                for(const Bounds &b: m_bounds) {
                    if(chr >= b.cmin && chr <= b.cmax)
                        return !m_invert;
                }

                return m_invert;
            } else
                return false;
        }

    private:
        List<Bounds> m_bounds;
    };

    class PatternNodeMultiChar : public PatternNode
    {
    public:
        PatternNodeMultiChar() = default;
        PatternNodeMultiChar(m::String &&str) : m_chars(str) {}

        virtual bool matches(CStringIterator &it, int &len) override
        {
            if(--len >= 0) {
                char chr = *(it++);

                for(char c: m_chars) {
                    if(chr == c)
                        return !m_invert;
                }

                return m_invert;
            } else
                return false;
        }

    private:
        String m_chars;
    };

    class PatternNodeAny : public PatternNode
    {
    public:
        virtual bool matches(CStringIterator &it, int &len) override
        {
            return --len >= 0;
        }
    };

    class PatternNodeEnd : public PatternNode
    {
    public:
        virtual bool matches(CStringIterator &it, int &len) override
        {
            return true;
        }

        virtual SpecialNodeType specialType() const override
        {
            return kSNT_End;
        }
    };

    class StackEntry
    {
    public:
        StackEntry() {}
        StackEntry(PatternNode *n, CStringIterator cit, int remLen) : node(n), it(cit), remainingLen(remLen), childId(0) {}

        PatternNode *node;
        CStringIterator it;
        int remainingLen;
        int childId;
    };

    static bool isInternalChar(char chr)
    {
        return chr == '[' || chr == ']' || chr == '(' || chr == ')' || chr == '.' || chr == '?' ||
               chr == '*' || chr == '+' || chr == '-' || chr == '^' || chr == '$';
    }

    static bool specialRange(char kind, List<PatternNodeMultiRange::Bounds> &dst, bool &containsRange)
    {
        kind = tolower(kind);

        if(kind == 's') {
            dst.add(PatternNodeMultiRange::Bounds(' '));
            dst.add(PatternNodeMultiRange::Bounds('\t'));
            dst.add(PatternNodeMultiRange::Bounds('\r'));
            dst.add(PatternNodeMultiRange::Bounds('\n'));
            dst.add(PatternNodeMultiRange::Bounds('\v'));
            dst.add(PatternNodeMultiRange::Bounds('\f'));
        } else if(kind == 'd') {
            dst.add(PatternNodeMultiRange::Bounds('0', '9'));
            containsRange = true;
        } else if(kind == 'l') {
            dst.add(PatternNodeMultiRange::Bounds('a', 'z'));
            containsRange = true;
        } else if(kind == 'p') {
            dst.add(PatternNodeMultiRange::Bounds('.'));
            dst.add(PatternNodeMultiRange::Bounds(','));
            dst.add(PatternNodeMultiRange::Bounds(';'));
            dst.add(PatternNodeMultiRange::Bounds(':'));
            dst.add(PatternNodeMultiRange::Bounds('!'));
            dst.add(PatternNodeMultiRange::Bounds('?'));
        } else if(kind == 'u') {
            dst.add(PatternNodeMultiRange::Bounds('A', 'Z'));
            containsRange = true;
        } else if(kind == 'w') {
            dst.add(PatternNodeMultiRange::Bounds('A', 'Z'));
            dst.add(PatternNodeMultiRange::Bounds('a', 'z'));
            dst.add(PatternNodeMultiRange::Bounds('0', '9'));
            containsRange = true;
        } else if(kind == 'x') {
            dst.add(PatternNodeMultiRange::Bounds('A', 'F'));
            dst.add(PatternNodeMultiRange::Bounds('a', 'f'));
            dst.add(PatternNodeMultiRange::Bounds('0', '9'));
            containsRange = true;
        } else
            return false;

        return true;
    }

    static PatternNode *makeNodeFromBoundList(List<PatternNodeMultiRange::Bounds> &ranges, bool hasRange, bool inv)
    {
        mDebugAssert(!ranges.isEmpty(), "makeNodeFromBoundList() called with empty bound list");
        PatternNode *ret;

        if(hasRange) {
            if(~ranges > 1)
                ret = new PatternNodeMultiRange(std::move(ranges));
            else
                ret = new PatternNodeCharRange(ranges[0].cmin, ranges[0].cmax);
        } else {
            if(~ranges > 1) {
                String str(~ranges);
                for(const PatternNodeMultiRange::Bounds &b: ranges)
                    str += b.cmin;

                ret = new PatternNodeMultiChar(std::move(str));
            } else
                ret = new PatternNodeChar(ranges[0].cmin);
        }

        if(inv)
            ret->invert();

        return ret;
    }

    static PatternParseError parseNode(CStringIterator &it, int &len, PatternNode *&ret)
    {
        if(*it == '.') { //Any
            it++;
            len--;
            ret = new PatternNodeAny;
        } else if(*it == '[') { //Range
            List<PatternNodeMultiRange::Bounds> ranges(2);
            bool hasRange = false; //true if 'ranges' contains at least one non-singleton range
            bool escape = false;
            bool invert;

            it++;
            len--;

            if(len > 0 && *it == '^') {
                invert = true;
                it++;
                len--;
            } else
                invert = false;

            while(len > 0) {
                if(escape) {
                    if(!specialRange(*it, ranges, hasRange))
                        ranges.add(PatternNodeMultiRange::Bounds(*it));

                    escape = false;
                    it++;
                    len--;
                } else if(*it == ']')
                    break;
                else if(len >= 3 && it[1] == '-') { //============> FIXME: This prevents the use of control caracters in ranges. But is it really needed?
                    if(isInternalChar(it[0]) || isInternalChar(it[2]))
                        return kPPE_MisplacedCtrlChar;

                    if(it[0] > it[2])
                        return kPPE_InvalidRange;

                    ranges.add(PatternNodeMultiRange::Bounds(it[0], it[2]));
                    it += 3;
                    len -= 3;

                    if(it[0] != it[2]) //Just in case someone does something stupid...
                        hasRange = true;
                } else {
                    if(*it == '%') //Escape sequence
                        escape = true;
                    else if(isInternalChar(*it))
                        return kPPE_MisplacedCtrlChar;
                    else
                        ranges.add(PatternNodeMultiRange::Bounds(*it));

                    it++;
                    len--;
                }
            }

            if(*it != ']')
                return kPPE_UnclosedBracket;

            if(escape)
                return kPPE_MisplacedEscape;

            it++;
            len--;
            ret = makeNodeFromBoundList(ranges, hasRange, invert);
        } else { //Exact match
            char chr = *it;

            if(chr == '%' && len >= 2) {
                //Escape
                chr = it[1];
                it += 2;
                len -= 2;

                List<PatternNodeMultiRange::Bounds> bounds;
                bool cr;

                if(specialRange(chr, bounds, cr)) {
                    ret = makeNodeFromBoundList(bounds, cr, false);
                    return kPPE_NoError;
                }
            } else {
                if(isInternalChar(chr))
                    return kPPE_MisplacedCtrlChar;

                it++;
                len--;
            }

            ret = new PatternNodeChar(chr);
        }

        return kPPE_NoError;
    }

    static int findParenthesisEnd(const char *start, int len)
    {
        start++;
        len--;

        int level = 0;
        bool escape = false;

        for(int ret = 0; ret < len; ret++) {
            if(escape)
                escape = false;
            else if(start[ret] == '%')
                escape = true;
            else if(start[ret] == '(')
                level++;
            else if(start[ret] == ')') {
                if(--level < 0)
                    return ret;
            }
        }

        return -1;
    }

    static void destroyPat(PatternNode *head)
    {
        if(head != nullptr) {
            List<PatternNode*> lst;
            head->dfs(lst);

            for(PatternNode *n: lst)
                delete n;
        }
    }

    static PatternParseError parsePat(CStringIterator it, int len, PatternNode *&head, PatternNode *&tail)
    {
        head = nullptr;
        tail = nullptr;
        PatternNode *addChildToThis = nullptr;

        while(len > 0) {
            PatternNode *newHead, *newTail;

            if(*it == '(') {
                int pLen = findParenthesisEnd(it, len);
                if(pLen <= 0) {
                    destroyPat(head);
                    head = nullptr;
                    tail = nullptr;

                    return (pLen < 0) ? kPPE_UnclosedParenthesis : kPPE_EmptyCapture;
                }

                PatternParseError ret = parsePat(++it, pLen, newHead, newTail);
                if(ret != kPPE_NoError) {
                    destroyPat(head);
                    head = nullptr;
                    tail = nullptr;
                    return (ret == kPPE_EmptyPattern) ? kPPE_EmptyCapture : ret;
                }

                it += pLen + 1;
                len -= pLen + 2;
            } else {
                PatternParseError ret = parseNode(it, len, newHead);
                if(ret != kPPE_NoError) {
                    destroyPat(head);
                    head = nullptr;
                    tail = nullptr;
                    return ret;
                }

                newTail = newHead;
            }

            if(addChildToThis != nullptr) {
                addChildToThis->addChild(newHead);
                addChildToThis = nullptr;
            }

            if(len > 0) {
                char chr = *it;
                uint8_t flags; //1 = add skip edge, 2 = start over

                if(chr == '?')
                    flags = 1;
                else if(chr == '*' || chr == '-')
                    flags = 3;
                else if(chr == '+')
                    flags = 2;
                else
                    flags = 0;

                if(flags & 1) { //Add skip edge
                    if(tail == nullptr) { //then head is null too
                        head = new PatternNodeEnd;
                        tail = head;
                    }

                    addChildToThis = tail;
                }

                if(flags & 2) //Add start over edge
                    newTail->addChild(newHead);

                if(flags != 0) {
                    it++;
                    len--;
                }
            }

            if(head == nullptr) { //then tail is null too
                head = newHead;
                tail = newTail;
            } else {
                tail->addChild(newHead);
                tail = newTail;
            }
        }

        if(head == nullptr)
            return kPPE_EmptyPattern;

        if(addChildToThis != nullptr) {
            PatternNode *end = new PatternNodeEnd; //This should be optimized out
            tail->addChild(end);
            addChildToThis->addChild(end);
            tail = end;
        }

        return kPPE_NoError;
    }
}

void m::PatternNode::optimize()
{
    m_marks |= kNM_OptimizePass;

    //Remove useless PatternNodeEnd
    bool redo;
    do {
        redo = false;

        for(int i = 0; i < ~m_next; i++) {
            PatternNode *pn = m_next[i];

            if(pn->specialType() == kSNT_End && ~pn->m_next > 0) {
                m_next.addAll(pn->m_next);
                m_next.remove(i);
                delete pn;
                redo = true;
                break;
            }
        }
    } while(redo);

    //Do the same for children
    for(PatternNode *pn : m_next) {
        if((pn->m_marks & kNM_OptimizePass) == 0)
            pn->optimize();
    }
}

m::Pattern::Pattern() : m_root(nullptr), m_flags(0), m_err(kPPE_NoError)
{
}

m::Pattern::~Pattern()
{
    destroyPat(m_root);

#ifdef M_PAT_TEST_MEMLEAK
    mAssert(g_nodeCount == 0, "node memory leak detected!!");
#endif
}

bool m::Pattern::compile(const char *sIt, int sLen)
{
    if(m_root != nullptr) {
        List<PatternNode*> toDelete;
        m_root->dfs(toDelete);

        for(PatternNode *pn: toDelete)
            delete pn;
    }

    if(sLen < 0) {
        sLen = 0;

        while(sIt[sLen] != 0)
            sLen++;
    }

    m_flags = 0;

    if(sLen > 0 && *sIt == '^') {
        m_flags |= kPF_Start;
        sIt++;
        sLen--;
    }

    if(sLen > 0 && sIt[sLen - 1] == '$') {
        m_flags |= kPF_End;
        sLen--;
    }

    PatternNode *tail;
    m_err = parsePat(sIt, sLen, m_root, tail);
    if(m_err != kPPE_NoError)
        return false;

    //m_root->optimize();
    if(tail->numChildren() > 0)
        tail->addChild(new PatternNodeEnd);

    return true;
}

const char *m::Pattern::parseErrorString() const
{
    switch(m_err) {
    case kPPE_NoError:             return "no error";
    case kPPE_UnclosedParenthesis: return "found opening parenthesis, but the closing one is missing";
    case kPPE_UnclosedBracket:     return "found opening bracket, but the closing one is missing";
    case kPPE_InvalidRange:        return "found invalid range";
    case kPPE_MisplacedEscape:     return "found misplaced character";
    case kPPE_EmptyCapture:        return "found empty capture";
    case kPPE_MisplacedCtrlChar:   return "found an unexpected control character";
    case kPPE_EmptyPattern:        return "pattern is empty";
    default:                       return "unknown error";
    }
}

int m::Matcher::matches()
{
    List<StackEntry> stack(4);
    PatternNode *node = m_pat->m_root;
    CStringIterator it = m_str.raw() + m_strPos;
    int remLen = m_str.length() - m_strPos;

    while(true) {
        if(node->matches(it, remLen)) {
            //Check children
            if(node->numChildren() <= 0)
                break; //End of pattern, and it seems to match!

            if(node->numChildren() > 1)
                stack.add(StackEntry(node, it, remLen));

            node = node->child(0);
        } else {
            //Roll back
            if(stack.isEmpty())
                return -1; //No more arrows :(

            node = stack.last().node;
            it = stack.last().it;
            remLen = stack.last().remainingLen;
            int cid = ++stack.last().childId; //At least one child to go, ALWAYS!

            if(cid + 1 >= node->numChildren())
                stack.remove(~stack - 1); //It's the last child to check...

            node = node->child(cid);
        }
    }

    return static_cast<int>(it - m_str.raw()) - m_strPos;
}

bool m::Matcher::next()
{
    m_captures.cleanup();
    m_starts.cleanup();
    m_ends.cleanup();

    int l;

    if(m_pat->m_flags & kPF_Start) {
        if(m_strPos > 0)
            return false;

        l = matches();
    } else {
        while(m_strPos < m_str.length()) {
            l = matches(); //We could pre-check m_pat->m_root to avoid useless expensive calls
            if(l >= 0)
                break;

            m_strPos++;
        }
    }

    if(l < 0)
        return false;

    if((m_pat->m_flags & kPF_End) != 0 && m_strPos + l < m_str.length())
        return false;

    m_starts.add(m_strPos);
    m_strPos += l;
    m_ends.add(m_strPos);
    m_captures.add(m_str.substr(m_starts[0], m_strPos));
    //TODO: Add other captures
    return true;
}
