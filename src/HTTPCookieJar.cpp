/* Copyright (C) 2020 BARBOTIN Nicolas
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

#include "mgpcl/HTTPCookieJar.h"

bool m::HTTPCookie::parse(const String &hdr)
{
    //Note: hdr has to be trimmed before this function
    //        is called.
    //
    //Note: this could be optimized by a single loop
    //        instead of several .indexOf()

    int end = hdr.indexOf(';');
    if(end < 0)
        end = hdr.length();

    int eq = hdr.indexOf('=');
    if(eq <= 0 || eq >= end)
        return false; //Missing the '=' character, or empty cookie name. Ignore.

    m_name = hdr.substr(0, eq);
    m_value = hdr.substr(eq + 1, end);

    m_expiry = std::numeric_limits<time_t>::max();
    m_maxAge = ~0;
    m_secure = false;
    m_httpOnly = false;
    m_sessionLocal = false;

    m_domain.cleanup();
    m_path.cleanup();

    int start = end + 1; //Ignore the ';'
    while(start < hdr.length()) {
        while(hdr[start] == ' ' || hdr[start] == '\t') { //Trim blanks
            if(++start >= hdr.length())
                return true;
        }

        end = hdr.indexOf(';', start);
        if(end < 0)
            end = hdr.length();

        eq = hdr.indexOf('=', start);
        if(eq > end)
            eq = end;

        String name(hdr.substr(start, eq));
        if(name.equalsIgnoreCase("Secure"_m))
            m_secure = true;
        else if(name.equalsIgnoreCase("HttpOnly"_m))
            m_httpOnly = true;
        else if(name.equalsIgnoreCase("Path"_m)) {
            if(eq + 1 < end && hdr[eq + 1] == '/') {
                if(eq + 2 < end && hdr[end - 1] == '/')
                    m_path = hdr.substr(eq + 1, end - 1); //Because of the use of 'startsWith' in 'isSuitableFor'
                else
                    m_path = hdr.substr(eq + 1, end);
            }
        } else if(name.equalsIgnoreCase("Domain"_m)) {
            if(eq + 1 < end) {
                if(hdr[eq + 1] == '.')
                    m_domain = hdr.substr(eq + 2, end); //Ignore the leading '.' character
                else
                    m_domain = hdr.substr(eq + 1, end);

                m_domain.toLower();
            }
        } else if(name.equalsIgnoreCase("Max-Age"_m)) {
            if(eq + 1 < end) {
                if(hdr[eq + 1] >= '0' && hdr[eq + 1] <= '9') {
                    m_maxAge = hdr.substr(eq + 1, end).toUInteger(); //This WILL NOT fail if there's a non-numeric character in here...
                    m_sessionLocal = false;
                }
            }
        } else if(name.equalsIgnoreCase("Expires"_m)) {
            //Gotta parse the date...
            if(eq + 1 < end) {
                String dateStr(hdr.substr(eq + 1, end));
                Date d;

                if(d.parseRFC6265_511(dateStr)) {
                    m_expiry = d.asUnixTimeGMT();
                    m_sessionLocal = false;
                }
            }
        }

        start = end + 1;
    }

    m_creation = Date::unixTime();
    return true;
}

bool m::HTTPCookie::isValid(time_t now) const
{
    return now < m_expiry && now - m_creation < static_cast<time_t>(m_maxAge);

}

bool m::HTTPCookie::isSuitableFor(const URL &u) const
{
    if(m_secure && !u.protocol().equalsIgnoreCase("https"_m))
        return false;

    if(!m_domain.isEmpty() && !u.host().lower().endsWith(m_domain))
        return false;

    if(m_path.isEmpty())
        return true;

    if(m_path.length() == 1 && m_path[0] == '/')
        return true; //Valid for the whole site

    return u.location().startsWith(m_path);
}

void m::HTTPCookie::serialize(DataSerializer &s) const
{
    s << m_creation << m_name << m_value << m_expiry << m_maxAge << m_domain << m_path << m_secure << m_httpOnly << m_sessionLocal;
}

void m::HTTPCookie::deserialize(DataDeserializer &s)
{
    s >> m_creation >> m_name >> m_value >> m_expiry >> m_maxAge >> m_domain >> m_path >> m_secure >> m_httpOnly >> m_sessionLocal;
}

void m::HTTPCookieJar::serialize(DataSerializer &s)
{
    time_t now = Date::unixTime();
    uint32_t count = 0;

    for(Pair &p: m_content) {
        if(p.value.isValid() && p.value.isValid(now))
            count++;
    }

    s << count;
    for(Pair &p : m_content) {
        if(p.value.isValid() && p.value.isValid(now))
            p.value.serialize(s);
    }
}

void m::HTTPCookieJar::deserialize(DataDeserializer &s)
{
    m_content.clear();

    time_t now = Date::unixTime();
    uint32_t count;
    s >> count;

    for(uint32_t i = 0; i < count; i++) {
        HTTPCookie c;
        c.deserialize(s);

        if(c.isValid() && c.isValid(now))
            m_content[c.name()] = c;
    }
}

bool m::HTTPCookieJar::parseAndPutCookie(const String &data)
{
    HTTPCookie c;
    if(!c.parse(data))
        return false;

    m_content[c.name()] = c;
    return true;
}
