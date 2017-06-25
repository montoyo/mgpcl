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
#include "Mem.h"
#include "Assert.h"
#include <cstdint>

namespace m
{
	//What I call a "cat-mouse" queue. Somebody probably invented it already, but I don't know how he called it.
	//Pushing a value will move the mouse forward
	//Poping a value will move the cat forward
	//When the cat eats the mouse, they start over
	//
	//The maximum number of elements that can be pushed is called backlog
	//When the backlog is reached, no more elements can be pushed and offer() fails (returns false)
	//If offerEx() is used, backlog is increased automatically and buffers are re-allocated.
	//Two buffers of size "backlog" are used, to avoid overlap and thus prefer the use of Mem::copy over Mem::move which should be a bit faster
	//
	//This queue implementation was designed to be faster than using Lists with .add() and .removeFirst(),
	//which would call Mem::move() everytime an element is popped. Here elements are copied to the secondary
	//buffer only if "the mouse" reaches the maximum size (the backlog). It has NO cost if elements are popped
	//faster than they are pushed, within the limits of the defined backlog, which can be changed at any moment
	//using .setBacklog(). Doing so will re-allocate the two buffers. One could also use offerEx() which is the same
	//as calling .setBacklog(.backlog() + .backlog() / 2) and then .offer() if the first call to .offer() fails (returns false).
	template<typename T> class Queue
	{
	public:
		Queue()
		{
			m_backlog = 2;
			m_size = 0;
			m_pos = 0;
			m_data = reinterpret_cast<T*>(new uint8_t[sizeof(T) * 4]);
			m_buffer = m_data + 2;
		}

		Queue(uint32_t bl)
		{
			m_backlog = bl;
			m_size = 0;
			m_pos = 0;
			m_data = reinterpret_cast<T*>(new uint8_t[sizeof(T) * (bl << 1)]);
			m_buffer = m_data + bl;
		}

		Queue(const Queue &src)
		{
			m_backlog = src.m_backlog;
			m_size = src.m_size;
			m_pos = 0;
			m_data = reinterpret_cast<T*>(new uint8_t[sizeof(T) * (m_backlog << 1)]);
			m_buffer = m_data + m_backlog;

			for(uint32_t i = 0; i < m_size; i++)
				new(m_data + i) T(src.m_data[src.m_pos + i]);
		}

		Queue(Queue &&src)
		{
			m_backlog = src.m_backlog;
			m_size = src.m_size;
			m_pos = src.m_pos;
			m_data = src.m_data;
			m_buffer = src.m_buffer;
			src.m_data = nullptr; //This is enough to "disable" destructor
		}

		~Queue()
		{
			if(m_data != nullptr) {
				for(uint32_t i = 0; i < m_size; i++)
					m_data[m_pos + i].~T();

				if(m_data < m_buffer)
					delete[] reinterpret_cast<uint8_t*>(m_data);
				else
					delete[] reinterpret_cast<uint8_t*>(m_buffer);
			}
		}

		bool offer(const T &o)
		{
			if(m_size >= m_backlog)
				return false; //Don't have space anymore; sorry :(

			if(m_pos + m_size >= m_backlog) {
				//Copy on next buffer
				for(uint32_t i = 0; i < m_size; i++) {
					new(m_buffer + i) T(std::move(m_data[m_pos + i]));
					m_data[m_pos + i].~T();
				}

				//Swap buffer and reset pos
				T *tmp = m_data;
				m_data = m_buffer;
				m_buffer = tmp;
				m_pos = 0;
			}

			new(m_data + m_pos + m_size++) T(o);
			return true;
		}

		void offerEx(const T &o)
		{
			if(m_size >= m_backlog) {
				//Realloc
				m_backlog += m_backlog >> 1; //backlog is at least two, so this would do 2 + (2 >> 1) = 2 + 1 = 3

				T *nptr = reinterpret_cast<T*>(new uint8_t[sizeof(T) * (m_backlog << 1)]);
				for(uint32_t i = 0; i < m_size; i++) {
					new(nptr + i) T(std::move(m_data[m_pos + i]));
					m_data[m_pos + i].~T();
				}

				if(m_data < m_buffer)
					delete[] reinterpret_cast<uint8_t*>(m_data);
				else
					delete[] reinterpret_cast<uint8_t*>(m_buffer);

				m_data = nptr;
				m_buffer = nptr + m_backlog;
			}

			if(m_pos + m_size >= m_backlog) {
				//Copy on next buffer
				for(uint32_t i = 0; i < m_size; i++) {
					new(m_buffer + i) T(std::move(m_data[m_pos + i]));
					m_data[m_pos + i].~T();
				}

				//Swap buffer and reset pos
				T *tmp = m_data;
				m_data = m_buffer;
				m_buffer = tmp;
				m_pos = 0;
			}

			new(m_data + m_pos + m_size++) T(o);
		}

		T &first()
		{
			mDebugAssert(m_size > 0, "tried to access first element of empty queue");
			return m_data[m_pos];
		}

		const T &first() const
		{
			mDebugAssert(m_size > 0, "tried to access first element of empty queue");
			return m_data[m_pos];
		}

		bool poll()
		{
			if(m_size == 0)
				return false;

			m_data[m_pos].~T();
			if(--m_size == 0)
				m_pos = 0;
			else
				m_pos++;

			return true;
		}

		bool isEmpty() const
		{
			return m_size == 0;
		}

		bool hasContent() const
		{
			return m_size > 0;
		}

		void clear()
		{
			for(uint32_t i = 0; i < m_size; i++)
				m_data[m_pos + i].~T();

			m_size = 0;
			m_pos = 0;
		}

		void setBacklog(uint32_t bl)
		{
			mDebugAssert(bl >= 2, "backlog must be at least 2");
			m_backlog = bl;

			T *nptr = reinterpret_cast<T*>(new uint8_t[sizeof(T) * (bl << 1)]);
			if(m_size > 0) {
				for(uint32_t i = 0; i < m_size; i++) {
					new(nptr + i) T(std::move(m_data[m_pos + i]));
					m_data[m_pos + i].~T();
				}

				m_pos = 0;
			}

			if(m_data < m_buffer)
				delete[] reinterpret_cast<uint8_t*>(m_data);
			else
				delete[] reinterpret_cast<uint8_t*>(m_buffer);

			m_data = nptr;
			m_buffer = nptr + bl;
		}

		uint32_t backlog() const
		{
			return m_backlog;
		}

		uint32_t size() const
		{
			return m_size;
		}

		Queue &operator = (const Queue &src)
		{
			if(m_data == src.m_data)
				return *this;

			if(m_backlog != src.m_backlog) {
				for(uint32_t i = 0; i < m_size; i++)
					m_data[m_pos + i].~T();

				if(m_data < m_buffer)
					delete[] reinterpret_cast<uint8_t*>(m_data);
				else
					delete[] reinterpret_cast<uint8_t*>(m_buffer);

				m_backlog = src.m_backlog;
				m_data = reinterpret_cast<T*>(new uint8_t[sizeof(T) * (m_backlog << 1)]);
				m_buffer = m_data + m_backlog;
			}

			m_size = src.m_size;
			m_pos = 0;

			for(uint32_t i = 0; i < m_size; i++)
				new(m_data + i) T(src.m_data[src.m_pos + i]);

			return *this;
		}

		Queue &operator = (Queue &&src)
		{
			for(uint32_t i = 0; i < m_size; i++)
				m_data[m_pos + i].~T();

			if(m_data < m_buffer)
				delete[] reinterpret_cast<uint8_t*>(m_data);
			else
				delete[] reinterpret_cast<uint8_t*>(m_buffer);

			m_backlog = src.m_backlog;
			m_size = src.m_size;
			m_pos = src.m_pos;
			m_data = src.m_data;
			m_buffer = src.m_buffer;
			src.m_data = nullptr; //This is enough to "disable" destructor
			return *this;
		}

	private:
		uint32_t m_backlog;
		uint32_t m_size;
		uint32_t m_pos;

		T *m_data;
		T *m_buffer;
	};
}
