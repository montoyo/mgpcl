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
#include "String.h"
#include <ctime>

namespace m
{
    class MGPCL_PREFIX Date
    {
    public:
        Date();
        Date(time_t unixt);
        ~Date();

        static time_t unixTime()
        {
            return ::time(nullptr);
        }

        static Date now();

        /* Returns the date/time as a string using the given format.
         * % +
         * s: Seconds
         * S: Seconds with a zero
         * m: Minutes
         * M: Minutes with a zero
         * h: Hour
         * H: Hour with a zero
         * d: Day of month
           * D: Day of month with a zero
           * o: Month number
         * O: Month number with a zero
         * y: Year number
         * n: Small month name
         * N: Complete month name
         * w: Small week day name
         * W: Complete week day name
         * f: Day of year number
         * %: %
         */
        String format(const String &frm);
        time_t asUnixTimeLocal() const; //Considers this Date fields as local
        time_t asUnixTimeGMT() const; //Considers this Date fields as GMT

        bool parseRFC6265_511(const String &str);

        int yearDay() const
        {
            return m_yDay;
        }

        int monthDay() const
        {
            return m_mDay;
        }

        int weekDay() const
        {
            return m_wDay;
        }

        int year() const
        {
            return m_year;
        }

        int month() const
        {
            return m_month;
        }

        int hour() const
        {
            return m_hour;
        }

        int minutes() const
        {
            return m_min;
        }

        int seconds() const
        {
            return m_sec;
        }

        void setSeconds(int sec)
        {
            m_sec = sec;
        }

        void setMinutes(int min)
        {
            m_min = min;
        }

        void setHour(int hour)
        {
            m_hour = hour;
        }

        void setMonthDay(int day)
        {
            m_mDay = day;
        }

        void setMonth(int month)
        {
            m_month = month;
        }

        void setYear(int year)
        {
            m_year = year;
        }

        void setWeekDay(int day)
        {
            m_wDay = day;
        }

        void setYearDay(int day)
        {
            m_yDay = day;
        }

        bool isDST() const
        {
            return m_dst;
        }

        void setDST(bool dst)
        {
            m_dst = dst;
        }

    private:
        int m_sec;
        int m_min;
        int m_hour;
        int m_mDay;
        int m_month;
        int m_year;
        int m_wDay;
        int m_yDay;
        bool m_dst;
    };
}
