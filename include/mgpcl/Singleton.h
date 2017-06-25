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

namespace m
{

	template<class T> class Singleton
	{
	public:
		static T &instance()
		{
			return *m_ptr;
		}

		static T &create()
		{
			if(m_ptr == nullptr)
				m_ptr = new T;

			return *m_ptr;
		}

		static void destroy()
		{
			if(m_ptr != nullptr) {
				delete m_ptr;
				m_ptr = nullptr;
			}
		}

	private:
		static T *m_ptr;
	};

	template<class T> T *Singleton<T>::m_ptr = nullptr;
	
	//If you reaaallllyy need a real singleton, here
	//it is; but I won't be held responsible for the
	//lack of performance in your program.
	template<class T> class RSingleton
	{
	public:
		static T &instance()
		{
			if(m_ptr == nullptr)
				m_ptr = new T;
			
			return *m_ptr;
		}

		static void destroy()
		{
			if(m_ptr != nullptr) {
				delete m_ptr;
				m_ptr = nullptr;
			}
		}

	private:
		static T *m_ptr;
	};

	template<class T> T *RSingleton<T>::m_ptr = nullptr;

}

