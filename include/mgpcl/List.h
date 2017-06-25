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
#include "Mem.h"
#include "Assert.h"
#include <functional>
#include <initializer_list>
#include <cstdint>

namespace m
{
	template<typename T, typename Size = int> class MGPCL_PREFIX List
	{
	public:
		List()
		{
			m_data = nullptr;
			m_size = Size(0);
			m_alloc = Size(0);
		}

		List(Size alloc)
		{
			m_alloc = alloc;
			m_size = 0;
			m_data = reinterpret_cast<T*>(new uint8_t[alloc * sizeof(T)]);
		}

		List(const List<T, Size> &src)
		{
			m_alloc = src.m_alloc;
			m_size = src.m_size;

			m_data = reinterpret_cast<T*>(new uint8_t[m_alloc * sizeof(T)]);
			for(Size i = Size(0); i < m_size; ++i)
				new(m_data + i) T(src.m_data[i]);
		}

		List(List<T, Size> &&src)
		{
			m_alloc = src.m_alloc;
			m_size = src.m_size;
			m_data = src.m_data;

			src.m_data = nullptr;
		}

		List(std::initializer_list<T> list)
		{
			m_alloc = Size(list.size());
			m_size = m_alloc;

			m_data = reinterpret_cast<T*>(new uint8_t[m_alloc * sizeof(T)]);
			for(Size i = Size(0); i < m_size; ++i)
				new(m_data + i) T(list.begin()[i]);
		}

		~List()
		{
			clear();
		}

		List<T, Size> &add(const T &obj)
		{
			if(m_size >= m_alloc)
				grow(Size(1));

			new(m_data + m_size++) T(obj);
			return *this;
		}

		List<T, Size> &add(T &&obj)
		{
			if(m_size >= m_alloc)
				grow(Size(1));

			new(m_data + m_size++) T(obj);
			return *this;
		}

		List<T, Size> &add(const T &obj, Size amount)
		{
			if(m_size + amount > m_alloc)
				grow(Size(amount));

			for(Size i = Size(0); i < amount; ++i)
				new(m_data + m_size + i) T(obj);

			m_size += amount;
			return *this;
		}

		List<T, Size> &addAll(const T *ray, Size rsz)
		{
			if(m_size + rsz > m_alloc)
				grow(rsz);

			for(Size i = Size(0); i < rsz; ++i)
				new(m_data + m_size + i) T(ray[i]);

			m_size += rsz;
			return *this;
		}

		List<T, Size> &addAll(const List<T, Size> &ray)
		{
			if(m_size + ray.m_size > m_alloc)
				grow(ray.m_size);

			for(Size i = Size(0); i < ray.m_size; ++i)
				new(m_data + m_size + i) T(ray.m_data[i]);

			m_size += ray.m_size;
			return *this;
		}

		T &get(Size id)
		{
			mDebugAssert(id >= Size(0) && id < m_size, "trying to get out of range value from List");
			return m_data[id];
		}

		const T &get(Size id) const
		{
			mDebugAssert(id >= Size(0) && id < m_size, "trying to get out of range value from List");
			return m_data[id];
		}

		void remove(Size id)
		{
			mDebugAssert(Size(id) >= Size(0) && id < m_size, "trying to remove out of range value from List");

			//destroy
			m_data[id].~T();

			//move
			--m_size;

			for(Size i = id; i < m_size; ++i) {
				new(m_data + i) T(std::move(m_data[i + 1]));
				m_data[i + 1].~T();
			}
		}

		bool removeFirst()
		{
			if(m_size <= 0)
				return false;

			m_data[0].~T();
			--m_size;

			for(Size i = Size(0); i < m_size; ++i) {
				new(m_data + i) T(std::move(m_data[i + 1]));
				m_data[i + 1].~T();
			}

			return true;
		}

		T *rawCopy() const
		{
			T *ret = reinterpret_cast<T*>(new uint8_t[m_size * sizeof(T)]);
			for(Size i = Size(0); i < m_size; ++i)
				new(ret + i) T(m_data[i]);

			//Hey this won't crash on delete, will it?
			return ret;
		}

		//This is slower than List<T, Size>(*this)
		List<T, Size> clone() const
		{
			List<T, Size> ret(m_size);
			for(Size i = Size(0); i < m_size; ++i)
				new(ret.m_data + i) T(m_data[i]);

			ret.m_size = m_size;
			return ret;
		}

		List<T, Size> sub(Size begin, int pEnd = -1) const
		{
			Size end;
			if(pEnd < 0) {
				Size sub(static_cast<Size>(-pEnd) - Size(1));

				if(sub >= m_size)
					end = Size(0);
				else
					end = m_size - sub;
			} else if(static_cast<Size>(pEnd) > m_size)
				end = m_size;
			else
				end = static_cast<Size>(pEnd);

			if(begin >= end)
				return List<T, Size>();

			Size len(end - begin);
			List<T, Size> ret(len);
			for(Size i = Size(0); i < len; ++i)
				new(ret.m_data + i) T(m_data[begin + i]);

			ret.m_size = len;
			return ret;
		}

		T *begin()
		{
			return m_data;
		}

		T *end()
		{
			return m_data + m_size;
		}

		const T *begin() const
		{
			return m_data;
		}

		const T *end() const
		{
			return m_data + m_size;
		}

		Size size() const
		{
			return m_size;
		}

		bool isEmpty() const
		{
			return m_size <= Size(0);
		}

		Size allocatedSpace() const
		{
			return m_alloc;
		}

		void reserve(Size sz)
		{
			if(m_alloc < sz)
				resize(sz);
		}

		//cmpFunc must return true if and only if a > b
		void insertionSort(std::function<bool(const T &, const T &)> cmpFunc)
		{
			uint8_t tmp_[sizeof(T)];
			T *tmp = reinterpret_cast<T*>(tmp_);

			for(Size i(0); i < m_size; ++i) {
				//Move entry to temporary buffer
				new(tmp) T(std::move(m_data[i]));
				m_data[i].~T();

				//Make some space
				Size j;
				for(j = i; j > Size(0) && cmpFunc(m_data[j - 1], *tmp); --j) {
					new(m_data + j) T(std::move(m_data[j - Size(1)]));
					m_data[j - Size(1)].~T();
				}

				//Fill da hole
				new(m_data + j) T(std::move(*tmp));
				tmp->~T();
			}
		}

		//When calling this version, T must implement "operator > (const T&) const"
		void insertionSort()
		{
			if(std::is_arithmetic<T>::value || std::is_pointer<T>::value) {
				for(Size i = Size(0); i < m_size; ++i) {
					T tmp = m_data[i];

					Size j;
					for(j = i; j > Size(0) && m_data[j - 1] > tmp; --j)
						m_data[j] = m_data[j - 1];

					m_data[j] = tmp;
				}
			} else {
				uint8_t tmp_[sizeof(T)];
				T *tmp = reinterpret_cast<T*>(tmp_);

				for(Size i(0); i < m_size; ++i) {
					//Move entry to temporary buffer
					new(tmp) T(std::move(m_data[i]));
					m_data[i].~T();

					//Make some space
					Size j;
					for(j = i; j > Size(0) && m_data[j - 1] > *tmp; --j) {
						new(m_data + j) T(std::move(m_data[j - Size(1)]));
						m_data[j - Size(1)].~T();
					}

					//Fill da hole
					new(m_data + j) T(std::move(*tmp));
					tmp->~T();
				}
			}
		}

		T &first()
		{
			mDebugAssert(m_size > Size(0), "trying to get first item of empty List");
			return m_data[0];
		}

		T &last()
		{
			mDebugAssert(m_size > Size(0), "trying to get last item of empty List");
			return m_data[m_size - Size(1)];
		}

		void clear()
		{
			if(m_data != nullptr) {
				for(Size i = Size(0); i < m_size; ++i)
					m_data[i].~T();

				delete[] reinterpret_cast<uint8_t*>(m_data); //i'm not quite sure; is this cast needed?
				m_alloc = Size(0);
				m_size = Size(0);
				m_data = nullptr;
			}
		}

		void cleanup()
		{
			for(Size i = Size(0); i < m_size; ++i)
				m_data[i].~T();

			m_size = Size(0);
		}

		List<T, Size> &pop(T &dst)
		{
			mDebugAssert(m_size > Size(0), "trying to pop item from empty List");
			dst = m_data[--m_size];
			m_data[m_size].~T();

			return *this;
		}

		T &operator[] (Size id)
		{
			return get(id);
		}

		const T &operator[] (Size id) const
		{
			return get(id);
		}

		List<T, Size> &operator << (const T &obj)
		{
			return add(obj);
		}

		List<T, Size> &operator << (T &&obj)
		{
			return add(obj);
		}

		List<T, Size> &operator >> (T &dst)
		{
			return pop(dst);
		}

		List<T, Size> &replace(Size id, const T &obj)
		{
			mDebugAssert(id >= Size(0) && id < m_size, "trying to replace out of range item from List");
			m_data[id] = obj; //is this okay?
			return *this;
		}

		List<T, Size> &replaceAll(Size id, const T *ray, Size rsz)
		{
			mDebugAssert(id >= Size(0) && id + rsz <= m_size, "trying to replace out of range item array from List");
			for(Size i = Size(0); i < rsz; ++i)
				m_data[id + i] = ray[i]; //is this okay?

			return *this;
		}

		List<T, Size> &replaceAll(Size id, const List<T, Size> &ray)
		{
			mDebugAssert(id >= Size(0) && id + ray.m_size <= m_size, "trying to replace out of range item List from List");
			for(Size i = Size(0); i < ray.m_size; ++i)
				m_data[id + i] = ray.m_data[i]; //is this okay?

			return *this;
		}

		List<T, Size> &insert(Size id, const T &obj)
		{
			mDebugAssert(id >= 0 && id <= m_size, "invalid insert position");
			if(m_size >= m_alloc)
				grow(Size(1));

			for(Size i = m_size; i > id; --i) {
				new(m_data + i) T(std::move(m_data[i - Size(1)]));
				m_data[i - Size(1)].~T();
			}

			new(m_data + id) T(obj);
			++m_size;
			return *this;
		}

		List<T, Size> &insertAll(Size id, const T *ray, Size rsz)
		{
			mDebugAssert(id >= 0 && id <= m_size, "invalid insert position");
			if(m_size + rsz > m_alloc)
				grow((m_size + rsz) - m_alloc);

			for(Size i = m_size - Size(1); i >= id; --i) {
				new(m_data + (i + rsz)) T(std::move(m_data[i]));
				m_data[i].~T();
			}

			for(Size i = Size(0); i < rsz; ++i)
				new(m_data + id + i) T(ray[i]);

			m_size += rsz;
			return *this;
		}

		List<T, Size> &insertAll(Size id, const List<T, Size> &ray)
		{
			mDebugAssert(id >= 0 && id <= m_size, "invalid insert position");
			if(m_size + ray.m_size > m_alloc)
				grow((m_size + ray.m_size) - m_alloc);

			for(Size i = m_size - Size(1); i >= id; --i) {
				new(m_data + (i + ray.size())) T(std::move(m_data[i]));
				m_data[i].~T();
			}

			for(Size i = Size(0); i < ray.m_size; ++i)
				new(m_data + id + i) T(ray.m_data[i]);

			m_size += ray.m_size;
			return *this;
		}

		List<T, Size> &operator = (const List<T, Size> &src)
		{
			if(src.m_data == m_data)
				return *this;

			clear();
			m_alloc = src.m_alloc;
			m_size = src.m_size;

			m_data = reinterpret_cast<T*>(new uint8_t[m_alloc * sizeof(T)]);
			for(Size i = Size(0); i < m_size; ++i)
				new(m_data + i) T(src.m_data[i]);

			return *this;
		}

		List<T, Size> &operator = (List<T, Size> &&src)
		{
			clear();
			m_alloc = src.m_alloc;
			m_size = src.m_size;
			m_data = src.m_data;

			src.m_data = nullptr;
			return *this;
		}

		List<T, Size> &operator = (const std::initializer_list<T> list)
		{
			clear();
			m_alloc = Size(list.size());
			m_size = m_alloc;

			m_data = reinterpret_cast<T*>(new uint8_t[m_alloc * sizeof(T)]);
			for(Size i = Size(0); i < m_size; ++i)
				new(m_data + i) T(list.begin()[i]);

			return *this;
		}

		//Has to stay as int to handle negative indexes
		int indexOf(std::function<bool(const T &)> compareFunc, int beg = 0) const
		{
			if(beg >= 0) {
				for(Size i = Size(beg); i < m_size; ++i) {
					if(compareFunc(m_data[i]))
						return static_cast<int>(i);
				}
			} else {
				for(Size i = Size(m_size + beg); i >= 0; --i) {
					if(compareFunc(m_data[i]))
						return static_cast<int>(i);
				}
			}

			return -1;
		}

		//Same here, has to stay int
		int indexOf(const T &src, int beg = 0) const
		{
			if(beg >= 0) {
				for(Size i = Size(beg); i < m_size; ++i) {
					if(m_data[i] == src)
						return static_cast<int>(i);
				}
			} else {
				for(Size i = Size(m_size + beg); i >= 0; --i) {
					if(m_data[i] == src)
						return static_cast<int>(i);
				}
			}

			return -1;
		}

		int lastIndexOf(const T &src) const
		{
			return indexOf(src, -1);
		}

	private:
		void grow(Size sz)
		{
			sz += m_size;
			Size add(m_alloc + (m_alloc >> Size(1)));

			if(add < sz)
				resize(sz);
			else
				resize(add);
		}

		void resize(Size sz)
		{
			m_alloc = sz;

			if(m_data == nullptr)
				m_data = reinterpret_cast<T*>(new uint8_t[sz * sizeof(T)]);
			else {
				T *nptr = reinterpret_cast<T*>(new uint8_t[sz * sizeof(T)]);
				for(Size i = Size(0); i < m_size; ++i) {
					new(nptr + i) T(std::move(m_data[i]));
					m_data[i].~T();
				}

				delete[] reinterpret_cast<uint8_t*>(m_data);
				m_data = nptr;
			}
		}

		T *m_data;
		Size m_size;
		Size m_alloc;
	};
}
