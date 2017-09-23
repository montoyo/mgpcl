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
#include "String.h"
#include "IOStream.h"
#include "SharedPtr.h"
#include "Config.h"

#ifndef MGPCL_NO_SSL

namespace m
{
	enum AESMode
	{
		kAESM_None = 0,
		kAESM_Encrypt,
		kAESM_Decrypt,
	};

	enum AESVersion
	{
		kAESV_None = 0,
		kAESV_128CBC,
		kAESV_192CBC,
		kAESV_256CBC,

		kAESV_Max //Not an AES version. KEEP AT END.
	};

	class AES
	{
	public:
		AES();
		AES(AESMode mode, AESVersion version, const uint8_t *key, const uint8_t *iv);
		AES(const AES &src);
		AES(AES &&src) noexcept;
		~AES();

		bool init(AESMode mode, AESVersion version, const uint8_t *key, const uint8_t *iv);
		void reset(const uint8_t *key, const uint8_t *iv);

		/* In encryption mode: dst should be able to handle at most 'srcLen + this->blockSize() - 1'
		 * In decryption mode: dst should be able to handle at most 'srcLen + this->blockSize()',
		 * except if this->blockSize() is 1, in which case 'srcLen' bytes is enough.
		 */
		int update(const uint8_t *src, uint32_t srcLen, uint8_t *dst, uint32_t dstLen);
		bool update(const uint8_t *src, uint32_t srcLen, uint8_t *dst, uint32_t *dstLen);

		/* In both encryption and decryption mode: 
		 * dst should be able to handle at most this->blockSize() bytes.
		 * Note that after this call, update() should not be called again,
		 * unless you call reset().
		 */
		int final(uint8_t *dst, uint32_t dstLen);
		bool final(uint8_t *dst, uint32_t *dstLen);

		uint32_t blockSize() const;
		uint32_t keySize() const;
		uint32_t ivSize() const;
		static uint32_t blockSize(AESVersion ver);
		static uint32_t keySize(AESVersion ver);
		static uint32_t ivSize(AESVersion ver);

		AESMode mode() const
		{
			return m_mode;
		}

		AESVersion version() const
		{
			return m_version;
		}

		bool isValid() const
		{
			return m_mode != kAESM_None && m_version != kAESV_None;
		}

		bool operator ! () const
        {
            return m_mode == kAESM_None || m_version == kAESV_None;
        }

		AES &operator = (const AES &src);
		AES &operator = (AES &&src) noexcept;

	private:
		AESMode m_mode;
		AESVersion m_version;
		void *m_aes_;
	};

	template<class RefCnt>
	class TAESOutputStream : public OutputStream
	{
		M_NON_COPYABLE_T(TAESOutputStream, RefCnt)

	public:
		TAESOutputStream()
		{
			m_pos = 0;
			m_bufSz = 8192;
			m_buf = new uint8_t[8192];
			m_finalized = false;
		}

		TAESOutputStream(const SharedPtr<OutputStream, RefCnt> &child) : m_out(child)
		{
			m_pos = 0;
			m_bufSz = 8192;
			m_buf = new uint8_t[8192];
			m_finalized = false;
		}

		TAESOutputStream(AESMode mode, AESVersion version, const uint8_t *key, const uint8_t *iv) : m_aes(mode, version, key, iv)
		{
			m_pos = 0;
			m_bufSz = 8192;
			m_buf = new uint8_t[8192];
			m_finalized = false;
		}

		TAESOutputStream(AESMode mode, AESVersion version, const uint8_t *key, const uint8_t *iv, const SharedPtr<OutputStream, RefCnt> &child) : m_aes(mode, version, key, iv), m_out(child)
		{
			m_pos = 0;
			m_bufSz = 8192;
			m_buf = new uint8_t[8192];
			m_finalized = false;
		}

		TAESOutputStream(TAESOutputStream<RefCnt> &&src) noexcept : m_aes(std::move(src.m_aes)), m_out(std::move(src.m_out))
		{
			m_pos = src.m_pos;
			m_bufSz = src.m_bufSz;
			m_buf = src.m_buf;
			m_finalized = src.m_finalized;

			src.m_buf = nullptr;
		}

		~TAESOutputStream() override
		{
			if(m_buf != nullptr)
				delete[] m_buf;
		}

		bool init(AESMode mode, AESVersion version, const uint8_t *key, const uint8_t *iv)
		{
			if(m_bufSz < AES::blockSize(version) + 1)
				return false;

			return m_aes.init(mode, version, key, iv);
		}

		bool isValid() const
		{
			return !m_out.isNull() && m_aes.isValid();
		}

        bool operator ! () const
        {
            return m_out.isNull() || !m_aes;
        }

		AES &aes()
		{
			return m_aes;
		}

		const AES &aes() const
		{
			return m_aes;
		}

		int write(const uint8_t *src, int sz) override
		{
			uint32_t remaining = static_cast<uint32_t>(sz);

			while(remaining > 0) {
				uint32_t cpSz = m_bufSz - m_aes.blockSize();
				if(remaining < cpSz)
					cpSz = remaining;

				int enc = m_aes.update(src, cpSz, m_buf, m_bufSz);
				if(enc < 0)
					return -1; //Encryption failed

				if(writeBuf(enc))
					return -1; //Writing to output failed

				if(m_finalized)
					m_finalized = false;

				m_pos += static_cast<uint64_t>(cpSz);
				remaining -= cpSz;
				src += cpSz;
			}

			return sz;
		}

		uint64_t pos() override
		{
			return m_pos;
		}

		bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
		{
			return false;
		}

		bool seekSupported() const override
		{
			return false;
		}

		bool flush() override
		{
			return m_out->flush();
		}

        //You need to call .final() or .finalAndFlush() before closing
		void close() override
		{
			m_out->close();
		}

		//TODO: Could be nice to keep an internal copy of key and iv
        //to avoid passing them again to .final() or .finalAndFlush()
		bool final(uint8_t *key, uint8_t *iv)
		{
			if(m_finalized)
				return true;

			int enc = m_aes.final(m_buf, m_bufSz);
			if(enc < 0)
				return false;

			m_aes.reset(key, iv);
			m_pos = 0;
			m_finalized = true;
			return !writeBuf(enc);
		}

		bool finalAndFlush(uint8_t *key, uint8_t *iv)
		{
			return final(key, iv) && m_out->flush();
		}

		//Call this after .init()!!
		bool setBufferSize(uint32_t bs)
		{
			if(!m_aes.isValid())
				return false;

			if(bs < m_aes.blockSize() + 1)
				return false;

			delete[] m_buf;
			m_bufSz = bs;
			m_buf = new uint8_t[bs];
			return true;
		}

		const SharedPtr<OutputStream, RefCnt> &child() const
		{
			return m_out;
		}

		void setChild(const SharedPtr<OutputStream, RefCnt> &child)
		{
			m_out = child;
		}

		bool hasChild() const
		{
			return !m_out.isNull();
		}

		TAESOutputStream<RefCnt> operator =(TAESOutputStream<RefCnt> &&src) noexcept
		{
			m_aes = std::move(src.m_aes);
			m_out = std::move(src.m_out);
			m_pos = src.m_pos;
			m_bufSz = src.m_bufSz;
			delete[] m_buf;
			m_buf = src.m_buf;
			m_finalized = src.m_finalized;
			src.m_buf = nullptr;

			return *this;
		}

	private:
		bool writeBuf(int sz)
		{
			uint8_t *buf = m_buf;

			while(sz > 0) {
				int wr = m_out->write(buf, sz);
				if(wr <= 0)
					return true;

				sz -= wr;
				buf += wr;
			}

			return false;
		}

		AES m_aes;
		SharedPtr<OutputStream, RefCnt> m_out;

		uint64_t m_pos;
		uint32_t m_bufSz;
		uint8_t *m_buf;
		bool m_finalized;
	};

	template<class RefCnt>
	class TAESInputStream : public InputStream
	{
		M_NON_COPYABLE_T(TAESInputStream, RefCnt)

	public:
		TAESInputStream()
		{
			m_numBlocks = 32;
			m_outAvail = 0;
			m_inBuf = nullptr;
			m_outBuf = nullptr;
			m_reachedEOF = false;
		}

		TAESInputStream(const SharedPtr<InputStream, RefCnt> &child) : m_in(child)
		{
			m_numBlocks = 32;
			m_outAvail = 0;
			m_inBuf = nullptr;
			m_outBuf = nullptr;
			m_reachedEOF = false;
		}

		TAESInputStream(AESMode mode, AESVersion ver, const uint8_t *key, const uint8_t *iv)
		{
			m_numBlocks = 32;
			m_outAvail = 0;
			m_inBuf = nullptr;
			m_outBuf = nullptr;
			m_reachedEOF = false;
			init(mode, ver, key, iv);
		}

		TAESInputStream(AESMode mode, AESVersion ver, const uint8_t *key, const uint8_t *iv, const SharedPtr<InputStream, RefCnt> &child) : m_in(child)
		{
			m_numBlocks = 32;
			m_outAvail = 0;
			m_inBuf = nullptr;
			m_outBuf = nullptr;
			m_reachedEOF = false;
			init(mode, ver, key, iv);
		}

		~TAESInputStream() override
		{
			if(m_inBuf != nullptr)
				delete[] m_inBuf;

			if(m_outBuf != nullptr)
				delete[] m_outBuf;
		}

		int read(uint8_t *dst, int sz) override
		{
			uint32_t toRead = static_cast<uint32_t>(sz);
			while(toRead > 0) {
				int err = fillOutBufIfNeeded();
				if(err <= 0)
					return err;

				if(m_outAvail < toRead) {
					Mem::copy(dst, m_outBuf, static_cast<size_t>(m_outAvail));
					dst += m_outAvail;
					toRead -= m_outAvail;
					m_outAvail = 0; //We just took everything; no more data available
				} else {
					Mem::copy(dst, m_outBuf, static_cast<size_t>(toRead));
					m_outAvail -= toRead;
					Mem::move(m_outBuf, m_outBuf + toRead, static_cast<size_t>(m_outAvail)); //Move everything at the beginning
					return sz; //Output buffer has been filled. Our job here, is done.
				}
			}

			return sz;
		}

		uint64_t pos() override
		{
			return 0ULL;
		}

		bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
		{
			return false;
		}

		bool seekSupported() const override
		{
			return false;
		}

		void close() override
		{
			m_in->close();
		}

		const SharedPtr<InputStream, RefCnt> &child() const
		{
			return m_in;
		}

		void setChild(const SharedPtr<InputStream, RefCnt> &child)
		{
			if(m_in != child) {
				m_in = child;
				m_reachedEOF = false;
			}
		}

		bool hasChild() const
		{
			return !m_in.isNull();
		}

		bool init(AESMode mode, AESVersion version, const uint8_t *key, const uint8_t *iv)
		{
			if(m_numBlocks < 1)
				return false;

			if(!m_aes.init(mode, version, key, iv))
				return false;

			if(m_inBuf != nullptr)
				delete[] m_inBuf;

			if(m_outBuf != nullptr)
				delete[] m_outBuf;

			uint32_t bsz = m_numBlocks * m_aes.blockSize();
			m_inBuf = new uint8_t[bsz];
			m_outBuf = new uint8_t[bsz + m_aes.blockSize()];
			return true;
		}

		void setBuffersNumBlocks(uint32_t nb)
		{
			if(m_numBlocks != nb) { //Avoid a re-allocation
				m_numBlocks = nb;

				if(m_aes.isValid()) {
					if(m_inBuf != nullptr)
						delete[] m_inBuf;

					if(m_outBuf != nullptr)
						delete[] m_outBuf;

					uint32_t bsz = m_numBlocks * m_aes.blockSize();
					m_inBuf = new uint8_t[bsz];
					m_outBuf = new uint8_t[bsz + m_aes.blockSize()];
				}
			}
		}

		bool isValid() const
		{
			return !m_in.isNull() && m_aes.isValid();
		}

        bool operator ! () const
        {
            return m_in.isNull() || !m_aes;
        }

		AES &aes()
		{
			return m_aes;
		}

		const AES &aes() const
		{
			return m_aes;
		}

	private:
		int fillOutBufIfNeeded()
		{
			while(m_outAvail == 0) {
				if(m_reachedEOF)
					return 0;

				uint32_t rsz = m_aes.blockSize() * m_numBlocks;
				int rd = m_in->read(m_inBuf, static_cast<int>(rsz));
				if(rd < 0)
					return rd; //Reading error

				if(rd == 0) {
					//Finish it!
					m_reachedEOF = true;
					rd = m_aes.final(m_outBuf, rsz + m_aes.blockSize());
					if(rd < 0)
						return -1;
				} else {
					//Simply update
					rd = m_aes.update(m_inBuf, static_cast<uint32_t>(rd), m_outBuf, rsz + m_aes.blockSize());
					if(rd < 0)
						return -1;
				}

				m_outAvail = static_cast<uint32_t>(rd);
			}

			return 1;
		}

		AES m_aes;
		SharedPtr<InputStream, RefCnt> m_in;

		uint32_t m_numBlocks;
		uint32_t m_outAvail;
		uint8_t *m_inBuf;
		uint8_t *m_outBuf;
		bool m_reachedEOF;
	};

	typedef TAESOutputStream<RefCounter> AESOutputStream;
	typedef TAESOutputStream<AtomicRefCounter> MTAESOutputStream;
	typedef TAESInputStream<RefCounter> AESInputStream;
	typedef TAESInputStream<AtomicRefCounter> MTAESInputStream;
}

#endif
