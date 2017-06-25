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

#include "mgpcl/Date.h"
#include "mgpcl/Mem.h"

static const char *g_sMonth[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static const char *g_cMonth[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
static const char *g_sDay[]   = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char *g_cDay[]   = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

m::Date::Date()
{
	Mem::zero(this, sizeof(Date));
}

m::Date::Date(time_t unixt)
{
#if (defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__ != 0) || (defined(__STDC_SECURE_LIB__) && __STDC_WANT_SECURE_LIB__ != 0)
	struct tm tms;
	struct tm *t = &tms;
	localtime_s(t, &unixt);
#else
	struct tm *t = localtime(&unixt);
#endif

	m_sec   = t->tm_sec;
	m_min   = t->tm_min;
	m_hour  = t->tm_hour;
	m_mDay  = t->tm_mday;
	m_month = t->tm_mon  + 1;
	m_year  = t->tm_year + 1900;
	m_wDay  = t->tm_wday;
	m_yDay  = t->tm_yday;
	m_dst   = t->tm_isdst != 0;
}

m::Date::~Date()
{
}

m::Date m::Date::now()
{
	return Date(time(nullptr));
}

m::String m::Date::format(const String &frm)
{
	return String::vformat([this] (char chr, VAList*) -> String {
		switch(chr) {
			case 's': return String::fromInteger(m_sec);
			case 'S': return m_sec < 10 ? String('0', 1) + String::fromInteger(m_sec) : String::fromInteger(m_sec);
			case 'm': return String::fromInteger(m_min);
			case 'M': return m_min < 10 ? String('0', 1) + String::fromInteger(m_min) : String::fromInteger(m_min);
			case 'h': return String::fromInteger(m_hour);
			case 'H': return m_hour < 10 ? String('0', 1) + String::fromInteger(m_hour) : String::fromInteger(m_hour);

			case 'd': return String::fromInteger(m_mDay);
			case 'D': return m_mDay < 10 ? String('0', 1) + String::fromInteger(m_mDay) : String::fromInteger(m_mDay);
			case 'o': return String::fromInteger(m_month);
			case 'O': return m_month < 10 ? String('0', 1) + String::fromInteger(m_month) : String::fromInteger(m_month);

			case 'n': return String(g_sMonth[m_month - 1]);
			case 'N': return String(g_cMonth[m_month - 1]);
			case 'w': return String(g_sDay[m_wDay]);
			case 'W': return String(g_cDay[m_wDay]);

			case 'y': return String::fromInteger(m_year);
			case 'f': return String::fromInteger(m_yDay);

			default: return String();
		}
	}, frm.raw(), nullptr);
}

time_t m::Date::asUnixTimeLocal() const
{
	struct tm t;

	t.tm_sec   = m_sec;
	t.tm_min   = m_min;
	t.tm_hour  = m_hour;
	t.tm_mday  = m_mDay;
	t.tm_mon   = m_month - 1;
	t.tm_year  = m_year - 1900;
	t.tm_isdst = m_dst ? 1 : 0;

	return mktime(&t);
}

time_t m::Date::asUnixTimeGMT() const
{
	time_t real = time(nullptr);
	time_t fake;
	struct tm t;

	{
#if (defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__ != 0) || (defined(__STDC_SECURE_LIB__) && __STDC_WANT_SECURE_LIB__ != 0)
		struct tm *tmp = &t;
		gmtime_s(tmp, &real);
#else
		struct tm *tmp = gmtime(&real);
#endif

		fake = mktime(tmp);
	}

	t.tm_sec = m_sec;
	t.tm_min = m_min;
	t.tm_hour = m_hour;
	t.tm_mday = m_mDay;
	t.tm_mon = m_month - 1;
	t.tm_year = m_year - 1900;
	t.tm_isdst = m_dst ? 1 : 0;

	time_t ret = mktime(&t);
	if(real >= fake)
		ret += real - fake;
	else
		ret -= fake - real;

	return ret;
}

bool m::Date::parseRFC6265_511(const String &str)
{
	bool timeSet = false;
	bool mdaySet = false;
	bool monthSet = false;
	bool yearSet = false;

	int start = 0;
	while(start < str.length()) {
		while((str[start] < '0' || str[start] > '9') && (str[start] < 'a' || str[start] > 'z') && (str[start] < 'A' || str[start] > 'Z') && str[start] != ':') {
			if(++start >= str.length())
				break;
		}

		if(start >= str.length())
			break;

		int end = start;
		while((str[end] >= '0' && str[end] <= '9') || (str[end] >= 'a' && str[end] <= 'z') || (str[end] >= 'A' && str[end] <= 'Z') || str[end] == ':') {
			if(++end >= str.length())
				break;
		}

		bool changed = false;
		if(!timeSet) {
			bool matchesHMS = true;

			{
				const char *pattern = "#?:#?:#?";
				int patternI = 0;
				int strI = start;

				while(pattern[patternI] != 0 && strI < end) {
					char c = pattern[patternI++];

					if(c == '#') {
						if(str[strI] < '0' || str[strI] > '9') {
							matchesHMS = false;
							break;
						}

						strI++;
					} else if(c == '?') {
						if(str[strI] >= '0' && str[strI] <= '9')
							strI++;
					} else if(c == ':') {
						if(str[strI] != ':') {
							matchesHMS = false;
							break;
						}

						strI++;
					}
				}

				if(pattern[patternI] != 0)
					matchesHMS = false;
			}

			if(matchesHMS) {
				int spos = start;
				int nums[3] = { 0, 0, 0 };

				for(int i = 0; i < 3; i++) {
					while(spos < end && str[spos] != ':')
						nums[i] = nums[i] * 10 + static_cast<int>(str[spos++] - '0');

					if(nums[i] < 0 || nums[i] >= 60)
						return false;

					spos++;
				}

				m_hour = nums[0];
				m_min = nums[1];
				m_sec = nums[2];
				timeSet = true;
				changed = true;
			}
		}

		if(!changed && (!mdaySet || !yearSet)) {
			bool isNumeric = true;
			int parsed = 0;

			for(int i = start; i < end; i++) {
				if(str[i] >= '0' && str[i] <= '9')
					parsed = parsed * 10 + static_cast<int>(str[i] - '0');
				else {
					isNumeric = false;
					break;
				}
			}

			if(isNumeric) {
				if(mdaySet) {
					//Well... then it's the year
					if(parsed >= 70 && parsed <= 99)
						parsed += 1900;
					else if(parsed >= 0 && parsed <= 69)
						parsed += 2000;

					if(parsed <= 1600)
						return false;

					m_year = parsed;
					yearSet = true;
				} else {
					//It's the month day
					if(parsed < 1 || parsed > 31)
						return false;

					m_mDay = parsed;
					mdaySet = true;
				}

				changed = true;
			}
		}

		if(!changed && !monthSet) {
			if(end - start == 3) {
				const char *months[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };
				int idx = -1;

				for(int i = 0; i < 12; i++) {
					if(tolower(str[start + 0]) == months[i][0] && tolower(str[start + 1]) == months[i][1] && tolower(str[start + 2]) == months[i][2]) {
						idx = i;
						break;
					}
				}

				if(idx >= 0) {
					m_month = idx + 1;
					monthSet = true;
					changed = true; //Quite useless now...
				}
			}
		}

		if(timeSet && mdaySet && monthSet && yearSet)
			return true;

		start = end;
	}

	return false;
}
