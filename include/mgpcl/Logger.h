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
#include "Enums.h"
#include "VAList.h"

#define mlogger (*::m::Logger::instance())
#define M_LOG __FILE__, __LINE__

namespace m
{
	class Logger
	{
	public:
		virtual ~Logger()
		{
		}

		virtual void vlog(LogLevel level, const char *fname, int line, const char *format, VAList *lst) = 0;

		void log(LogLevel level, const char *fname, int line, const char *format, ...)
		{
			VAList lst;
			va_start(lst.list, format);
			vlog(level, fname, line, format, &lst);
			va_end(lst.list);
		}

		void debug(const char *fname, int line, const char *format, ...)
		{
			VAList lst;
			va_start(lst.list, format);
			vlog(LogLevel::Debug, fname, line, format, &lst);
			va_end(lst.list);
		}

		void info(const char *fname, int line, const char *format, ...)
		{
			VAList lst;
			va_start(lst.list, format);
			vlog(LogLevel::Info, fname, line, format, &lst);
			va_end(lst.list);
		}

		void warning(const char *fname, int line, const char *format, ...)
		{
			VAList lst;
			va_start(lst.list, format);
			vlog(LogLevel::Warning, fname, line, format, &lst);
			va_end(lst.list);
		}

		void error(const char *fname, int line, const char *format, ...)
		{
			VAList lst;
			va_start(lst.list, format);
			vlog(LogLevel::Error, fname, line, format, &lst);
			va_end(lst.list);
		}

		/* BASIC LOGGER SINGLETONING SYSTEM
		 * 
		 * At startup, Logger::instance() is always null.
		 * You have to set it up manually, with, for instance,
		 * BasicLogger which will print into the console.
		 * 
		 * Note the presence of the 'mlogger' macro, which is
		 * basically a shortcut for '*Logger::instance()'
		 * 
		 * This is not thread safe, in order to improve speed.
		 * This shouldn't be a problem since most of the time,
		 * the program's logger is set at startup, before any
		 * other thread is started.
		 * 
		 * Also note this is YOUR job to care about the destruction
		 * of the logger. If Logger::instance() is not null, you can
		 * do something like 'delete Logger::setLoggerInstance(newLogger)'.
		 */

		static Logger *instance()
		{
			return m_instance;
		}

		static Logger *setLoggerInstance(Logger *newl)
		{
			Logger *ret = m_instance;
			m_instance = newl;

			return ret;
		}

	private:
		static Logger *m_instance;
	};

	class NullLogger : public Logger
	{
	public:
		virtual ~NullLogger()
		{
		}

		void vlog(LogLevel level, const char *fname, int line, const char *format, VAList *lst) override
		{
		}
	};

}
