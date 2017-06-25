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
#include "Allocator.h"
#include "DataSerializer.h"

namespace m
{
	template<class Alloc> class TPacket;
	template<class Alloc> class TPrePacket;
	template<class Alloc> class TPacketReader;

	template<class Alloc> class TFPacket
	{
		friend class TPacket<Alloc>;
		friend class TPrePacket<Alloc>;
		friend class TPacketReader<Alloc>;

	public:
		TFPacket()
		{
			m_data = nullptr;
			m_len = 0;
		}

		uint32_t size() const
		{
			return m_len;
		}

		uint8_t *data() const
		{
			return m_data;
		}

		const Alloc &allocator() const
		{
			return m_al;
		}

		void destroy()
		{
			m_al.deallocate(m_data);
			m_data = nullptr;
		}

		bool isValid() const
		{
			return m_data != nullptr;
		}

		TFPacket<Alloc> duplicate() const
		{
			//TODO: Allow ref-counting packets to avoid this!
			uint8_t *cpy = m_al.allocate(m_len);
			Mem::copy(cpy, m_data, static_cast<size_t>(m_len));

			return TFPacket<Alloc>(m_al, cpy, m_len);
		}

	private:
		TFPacket(const Alloc &al, uint8_t *ptr, uint32_t sz) : m_al(al)
		{
			m_data = ptr;
			m_len = sz;
		}

		Alloc m_al;
		uint8_t *m_data;
		uint32_t m_len;
	};

	template<class Alloc> class TPacket : public DataSerializer
	{
	public:
		TPacket() : DataSerializer(Endianness::Big)
		{
			//Reserve 4 uninitialized bytes for packet size, and 4 more because the user is going to need them
			m_data = m_al.allocate(sizeof(uint32_t) + 4);
			m_alloc = sizeof(uint32_t) + 4;
			m_len = sizeof(uint32_t);
			m_pos = 0;
		}

		TPacket(uint32_t al) : DataSerializer(Endianness::Big)
		{
			//Reserve 4 uninitialized bytes for packet size
			m_alloc = sizeof(uint32_t) + al;
			m_data = m_al.allocate(m_alloc);
			m_len = sizeof(uint32_t);
			m_pos = 0;
		}

		TPacket(const TPacket<Alloc> &src) : DataSerializer(src), m_al(src.m_al)
		{
			m_alloc = src.m_alloc;
			m_len = src.m_len;
			m_pos = src.m_pos;

			if(m_alloc > 0) {
				m_data = m_al.allocate(m_alloc);

				if(m_len > sizeof(uint32_t))
					Mem::copy(m_data, src.m_data, m_len); //No need to copy the 4 first bytes, but it's not a problem if we do...
			}
		}

		TPacket(TPacket<Alloc> &&src) : DataSerializer(src), m_al(std::move(src.m_al))
		{
			m_data = src.m_data;
			m_alloc = src.m_alloc;
			m_len = src.m_len;
			m_pos = src.m_pos;
			src.m_data = nullptr;
		}

		~TPacket() override
		{
			if(m_data != nullptr)
				m_al.deallocate(m_data);
		}

		uint32_t pos() const
		{
			return m_pos;
		}

		uint32_t realPos() const
		{
			return m_pos + sizeof(uint32_t);
		}

		uint32_t size() const
		{
			//Size of the payload
			return m_len - sizeof(uint32_t);
		}

		uint32_t realSize() const
		{
			//Size of the packet that will be sent over the network
			return m_len;
		}

		uint32_t write(const uint8_t *data, uint32_t sz)
		{
			write(m_pos + sizeof(uint32_t), data, sz);
			m_pos += sz;
			return sz;
		}

		bool seek(int amount, SeekPos sp = SeekPos::Beginning)
		{
			switch(sp) {
			case SeekPos::Beginning:
				mDebugAssert(amount >= 0, "cannot seek backward from beginning");
				m_pos = static_cast<uint32_t>(amount);
				return true;

			case SeekPos::Relative:
				if(amount < 0) {
					uint32_t amnt = static_cast<uint32_t>(-amount);
					if(amnt > m_pos)
						return false;

					m_pos -= amnt;
				} else
					m_pos += static_cast<uint32_t>(amount);

				return true;

			case SeekPos::End:
				if(amount < 0) {
					uint32_t amnt = static_cast<uint32_t>(-amount);
					if(amnt > m_len - sizeof(uint32_t))
						return false;

					m_pos = m_len - sizeof(uint32_t) - amnt;
				} else
					m_pos = m_len - sizeof(uint32_t) + static_cast<uint32_t>(amount);

				return true;

			default:
				//Should never happen
				return false;
			}
		}

		void reserve(uint32_t len)
		{
			len += sizeof(uint32_t);

			if(len > m_alloc)
				grow(len);
		}

		TFPacket<Alloc> finalize()
		{
			if(m_alloc < 4)
				grow(4);

			if(m_len < 4)
				m_len = 4;

			//Write size at beginning. Always big endian.
			*reinterpret_cast<uint32_t*>(m_data) = ((m_len & 0x000000FF) << 24) | ((m_len & 0x0000FF00) << 8) | ((m_len & 0x00FF0000) >> 8) | ((m_len & 0xFF000000) >> 24);

			TFPacket<Alloc> ret(m_al, m_data, m_len);
			m_data = nullptr;
			return ret;
		}

		TPacket<Alloc> &operator = (const TPacket<Alloc> &src)
		{
			if(m_data == src.m_data)
				return *this;

			if(m_data != nullptr)
				m_al.deallocate(m_data);

			m_ed = src.m_ed;
			m_stringMode = src.m_stringMode;
			m_al = src.m_al;
			m_alloc = src.m_alloc;
			m_len = src.m_len;
			m_pos = src.m_pos;

			if(src.m_data == nullptr)
				m_data = nullptr;
			else {
				if(m_alloc > 0) {
					m_data = m_al.allocate(m_alloc);

					if(m_len > sizeof(uint32_t))
						Mem::copy(m_data, src.m_data, m_len); //No need to copy the 4 first bytes, but it's not a problem if we do...
				}
			}

			return *this;
		}

		TPacket<Alloc> &operator = (TPacket<Alloc> &&src)
		{
			if(m_data != nullptr)
				m_al.deallocate(m_data);

			m_ed = src.m_ed;
			m_stringMode = src.m_stringMode;
			m_al = std::move(src.m_al);
			m_data = src.m_data;
			m_alloc = src.m_alloc;
			m_len = src.m_len;
			m_pos = src.m_pos;
			src.m_data = nullptr;
			return *this;
		}

	protected:
		void dsWrite(const uint8_t *data, int sz) override
		{
			uint32_t usz = static_cast<uint32_t>(sz);
			write(m_pos + sizeof(uint32_t), data, usz);
			m_pos += usz;
		}

		void write(uint32_t pos, const uint8_t *data, uint32_t len)
		{
			//Ensure capacity
			uint32_t end = pos + len;
			if(end > m_alloc)
				grow(end);

			//Replace or insert, we don't care
			Mem::copy(m_data + pos, data, len);
			if(end > m_len)
				m_len = end;
		}

		void grow(uint32_t nsz)
		{
			//Calls to grow() needs to check if nsz > m_alloc first
			m_alloc += m_alloc >> 1;
			if(m_alloc < nsz)
				m_alloc = nsz;

			if(m_data == nullptr)
				m_data = m_al.allocate(m_alloc);
			else
				m_data = m_al.reallocate(m_data, m_len, m_alloc);
		}

	private:
		Alloc m_al;
		uint8_t *m_data;
		uint32_t m_alloc;
		uint32_t m_len;
		uint32_t m_pos;
	};

	template<class Alloc> class TPrePacket
	{
	public:
		TPrePacket()
		{
			m_buf = nullptr;
			m_size = 0;
			m_pos = 0;
		}

		TPrePacket(uint32_t sz)
		{
			m_buf = m_al.allocate(sz);
			m_size = sz;
			m_pos = 0;
		}

		TPrePacket(const TPrePacket &src) : m_al(src.m_al)
		{
			m_size = src.m_size;
			m_pos = src.m_pos;

			if(src.m_buf == nullptr)
				m_buf = nullptr;
			else {
				m_buf = m_al.allocate(m_size);

				if(m_pos > 0)
					Mem::copy(m_buf, src.m_buf, m_pos);
			}
		}

		TPrePacket(TPrePacket &&src) : m_al(std::move(src.m_al))
		{
			m_buf = src.m_buf;
			m_size = src.m_size;
			m_pos = src.m_pos;
			src.m_buf = nullptr;
		}

		~TPrePacket()
		{
			if(m_buf != nullptr)
				m_al.deallocate(m_buf);
		}

		void setSize(uint32_t sz)
		{
			mDebugAssert(m_buf == nullptr, "cannot resize prepacket");
			m_buf = m_al.allocate(sz);
			m_size = sz;
		}

		uint32_t fill(const uint8_t *data, uint32_t sz)
		{
			if(m_pos >= m_size)
				return 0;

			uint32_t rem = m_size - m_pos;
			if(sz > rem)
				sz = rem;

			Mem::copy(m_buf, data, sz);
			m_pos += sz;
			return sz;
		}

		bool isReady() const
		{
			return m_pos >= m_size;
		}

		uint32_t size() const
		{
			return m_size;
		}

		TFPacket<Alloc> finalize()
		{
			mDebugAssert(m_size > 0 && m_pos >= m_size, "prepacket isn't finished");

			TFPacket<Alloc> ret(m_al, m_buf, m_size);
			m_buf = nullptr;
			m_size = 0;
			m_pos = 0;
			return ret;
		}

		TPrePacket<Alloc> &operator = (const TPrePacket<Alloc> &src)
		{
			if(m_buf == src.m_buf)
				return *this;

			if(m_buf != nullptr)
				m_al.deallocate(m_buf);

			m_al = src.m_al;
			m_size = src.m_size;
			m_pos = src.m_pos;

			if(src.m_buf == nullptr)
				m_buf = nullptr;
			else {
				m_buf = m_al.allocate(m_size);

				if(m_pos > 0)
					Mem::copy(m_buf, src.m_buf, m_pos);
			}

			return *this;
		}

		TPrePacket<Alloc> &operator = (TPrePacket<Alloc> &&src)
		{
			if(m_buf != nullptr)
				m_al.deallocate(m_buf);

			m_al = std::move(src.m_al);
			m_buf = src.m_buf;
			m_size = src.m_size;
			m_pos = src.m_pos;
			src.m_buf = nullptr;
			return *this;
		}

		bool isValid() const
		{
			return m_buf != nullptr;
		}

	private:
		Alloc m_al;
		uint8_t *m_buf;
		uint32_t m_size;
		uint32_t m_pos;
	};

	template<class Alloc> class TPacketReader : public DataDeserializer
	{
		M_NON_COPYABLE_T(TPacketReader, Alloc)

	public:
		TPacketReader() : DataDeserializer(Endianness::Big)
		{
			m_data = nullptr;
			m_size = 0;
			m_pos = 0;
		}

		TPacketReader(const TFPacket<Alloc> &pkt) : DataDeserializer(Endianness::Big), m_al(pkt.allocator())
		{
			m_data = pkt.data();
			m_size = pkt.size();
			m_pos = 0;
		}

		TPacketReader(const TPacketReader<Alloc> &&src) : DataDeserializer(src), m_al(std::move(src.m_al))
		{
			m_data = src.m_data;
			m_size = src.m_size;
			m_pos = src.m_pos;
			src.m_data = nullptr;
		}

		~TPacketReader() override
		{
			if(m_data != nullptr)
				m_al.deallocate(m_data);
		}

		uint32_t read(uint8_t *dst, uint32_t sz)
		{
			uint32_t remaining = m_size - m_pos;
			if(sz > remaining)
				sz = remaining;

			if(sz == 0)
				return 0;

			Mem::copy(dst, m_data + m_pos, sz);
			m_pos += sz;
			return sz;
		}

		uint32_t pos() const
		{
			return m_pos;
		}

		uint32_t size() const
		{
			return m_size;
		}

		bool isValid() const
		{
			return m_data != nullptr;
		}

		void set(const TFPacket<Alloc> &src)
		{
			if(m_data != nullptr)
				m_al.deallocate(m_data);

			m_al = src.allocator();
			m_data = src.data();
			m_size = src.size();
			m_pos = 0;
		}

		bool seek(int amount, SeekPos sp = SeekPos::Beginning)
		{
			switch(sp) {
			case SeekPos::Beginning:
				mDebugAssert(amount >= 0, "cannot seek backward from beginning");
				if(static_cast<uint32_t>(amount) > m_size)
					return false;

				m_pos = static_cast<uint32_t>(amount);
				return true;

			case SeekPos::Relative:
				if(amount < 0) {
					uint32_t amnt = static_cast<uint32_t>(-amount);
					if(amnt > m_pos)
						return false;

					m_pos -= amnt;
				} else {
					uint32_t amnt = static_cast<uint32_t>(amount);
					if(m_pos + amnt > m_size)
						return false;

					m_pos += amnt;
				}

				return true;

			case SeekPos::End:
				if(amount < 0) {
					uint32_t amnt = static_cast<uint32_t>(-amount);
					if(amnt > m_size)
						return false;

					m_pos = m_size - amnt;
				} else
					return false;

				return true;

			default:
				//Should never happen
				return false;
			}
		}

		TFPacket<Alloc> cancel()
		{
			mDebugAssert(m_data != nullptr, "can't cancel an invalid packet reader");

			TFPacket<Alloc> ret(m_al, m_data, m_size);
			m_data = nullptr;
			return ret;
		}

		TPacketReader<Alloc> &operator = (TPacketReader<Alloc> &&src)
		{
			if(m_data != nullptr)
				m_al.deallocate(m_data);

			m_al = std::move(src.m_al);
			m_data = src.m_data;
			m_size = src.m_size;
			m_pos = src.m_pos;
			src.m_data = nullptr;
			return *this;
		}

	protected:
		void dsRead(uint8_t *dst, int sz) override
		{
			if(m_pos + static_cast<uint32_t>(sz) > m_size) {
				//Woops! What do we do here?
				Mem::zero(dst, sz);
			} else {
				Mem::copy(dst, m_data + m_pos, sz);
				m_pos += static_cast<uint32_t>(sz);
			}
		}

	private:
		Alloc m_al;
		uint8_t *m_data;
		uint32_t m_size;
		uint32_t m_pos;
	};

	typedef TFPacket<Allocator<uint8_t>> FPacket;
	typedef TPacket<Allocator<uint8_t>> Packet;
	typedef TPrePacket<Allocator<uint8_t>> PrePacket;
	typedef TPacketReader<Allocator<uint8_t>> PacketReader;

}
