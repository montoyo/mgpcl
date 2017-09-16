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
#include "IOStream.h"
#include "DataSerializer.h"
#include "SharedPtr.h"
#include "Util.h"
#include "RefCounter.h"

namespace m
{
	template<class RefCnt> class MGPCL_PREFIX TDataInputStream : public InputStream, public DataDeserializer
	{
	public:
		//Constructor
		TDataInputStream()
		{
		}

		TDataInputStream(Endianness ed) : DataDeserializer(ed)
		{
		}

		TDataInputStream(SharedPtr<InputStream, RefCnt> child, Endianness ed = Endianness::Little) : DataDeserializer(ed), m_child(child)
		{
		}

		//Implemented functions
		int read(uint8_t *dst, int sz) override
		{
			return m_child->read(dst, sz);
		}

		bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
		{
			return m_child->seek(amount, sp);
		}

		bool seekSupported() const override
		{
			return m_child->seekSupported();
		}
		
		uint64_t pos() override
		{
			return m_child->pos();
		}

		void close() override
		{
			m_child->close();
		}

		SharedPtr<InputStream, RefCnt> child() const
		{
			return m_child;
		}

		void setChild(SharedPtr<InputStream, RefCnt> inputStream)
		{
			m_child = inputStream;
		}

	protected:
		void dsRead(uint8_t *dst, int sz) override
		{
			if(!IO::readFully(m_child.ptr(), dst, sz))
				throw IOException(RWAction::Reading);
		}

	private:
		SharedPtr<InputStream, RefCnt> m_child;
	};

	template<class RefCnt> class TDataOutputStream : public OutputStream, public DataSerializer
	{
	public:
		//Constructor
		TDataOutputStream()
		{
		}

		TDataOutputStream(Endianness ed) : DataSerializer(ed)
		{
		}

		TDataOutputStream(SharedPtr<OutputStream, RefCnt> child, Endianness ed = Endianness::Little) : DataSerializer(ed), m_child(child)
		{
		}

		//Implemented methods
		int write(const uint8_t *src, int sz) override
		{
			return m_child->write(src, sz);
		}

		uint64_t pos() override
		{
			return m_child->pos();
		}

		bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
		{
			return m_child->seek(amount, sp);
		}

		bool seekSupported() const override
		{
			return m_child->seekSupported();
		}

		bool flush() override
		{
			return m_child->flush();
		}

		void close() override
		{
			m_child->close();
		}

		SharedPtr<OutputStream, RefCnt> child() const
		{
			return m_child;
		}

		void setChild(SharedPtr<OutputStream, RefCnt> oStream)
		{
			m_child = oStream;
		}

	protected:
		void dsWrite(const uint8_t *src, int sz) override
		{
			if(!IO::writeFully(m_child.ptr(), src, sz))
				throw IOException(RWAction::Writing);
		}

	private:
		SharedPtr<OutputStream, RefCnt> m_child;
	};

	typedef TDataInputStream<RefCounter> DataInputStream;
	typedef TDataInputStream<AtomicRefCounter> MTDataInputStream;
	typedef TDataOutputStream<RefCounter> DataOutputStream;
	typedef TDataOutputStream<AtomicRefCounter> MTDataOutputStream;

}
