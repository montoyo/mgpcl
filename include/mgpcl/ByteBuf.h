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

#include <cstdint>
#include "IOStream.h"
#include "SharedPtr.h"
#include "Allocator.h"

namespace m
{
    template<class Alloc> class TByteBuf
    {
    public:
        TByteBuf()
        {
            m_data = nullptr;
            m_alloc = 0;
            m_len = 0;
        }

        TByteBuf(const TByteBuf<Alloc> &src)
        {
            m_alloc = src.m_alloc;
            m_len = src.m_len;
            m_data = m_al.allocate(m_alloc);
            Mem::copy(m_data, src.m_data, m_len);
        }

        TByteBuf(TByteBuf<Alloc> &&src)
        {
            m_alloc = src.m_alloc;
            m_len = src.m_len;
            m_data = src.m_data;

            src.m_data = nullptr;
        }

        TByteBuf(uint32_t sz)
        {
            m_alloc = sz;
            m_len = 0;
            m_data = m_al.allocate(sz);
        }

        ~TByteBuf()
        {
            if(m_data != nullptr)
                m_al.deallocate(m_data);
        }

        TByteBuf<Alloc> &operator = (const TByteBuf<Alloc> &src)
        {
            if(m_alloc != src.m_alloc) {
                if(m_data != nullptr)
                    m_al.deallocate(m_data);

                m_alloc = src.m_alloc;
                m_len = src.m_len;
                m_data = m_al.allocate(m_alloc);
                Mem::copy(m_data, src.m_data, m_len);
            } else {
                m_len = src.m_len;
                Mem::copy(m_data, src.m_data, m_len);
            }

            return *this;
        }

        TByteBuf<Alloc> &operator = (TByteBuf<Alloc> &&src)
        {
            if(m_data != nullptr)
                m_al.deallocate(m_data);

            m_alloc = src.m_alloc;
            m_len = src.m_len;
            m_data = src.m_data;
            
            src.m_data = nullptr;
            return *this;
        }

        template<class RefCnt> SharedPtr<InputStream, RefCnt> inputStream(); //can't write this now
        template<class RefCnt> SharedPtr<OutputStream, RefCnt> outputStream(); //can't write this now neither

        void clear()
        {
            if(m_data != nullptr)
                m_al.deallocate(m_data);

            m_alloc = 0;
            m_len = 0;
            m_data = nullptr;
        }

        void cleanup()
        {
            m_len = 0;
        }

        uint8_t *data()
        {
            return m_data;
        }

        const uint8_t *data() const
        {
            return m_data;
        }

        uint8_t *dataCopy() const
        {
            //Don't use Alloc here
            uint8_t *ret = new uint8_t[m_len];
            Mem::copy(ret, m_data, m_len);

            return ret;
        }

        uint32_t size() const
        {
            return m_len;
        }

        Alloc &allocator()
        {
            return m_al;
        }

        void ensureReachable(uint32_t pos)
        {
            if(pos > m_len) {
                if(pos > m_alloc)
                    grow(pos - m_len);

                while(m_len < pos)
                    m_data[m_len++] = 0;
            }
        }

        int write(uint32_t pos, const uint8_t *data, int len)
        {
            if(pos >= m_len) {
                //Ensure capacity
                uint32_t toAdd = (pos - m_len) + static_cast<uint32_t>(len);
                if(m_len + toAdd > m_alloc)
                    grow(toAdd);

                //Pad
                while(m_len < pos)
                    m_data[m_len++] = 0;

                //Insert
                Mem::copy(m_data + m_len, data, len);
                m_len += len;
            } else {
                //Make sure we have enough space
                uint32_t nlen = pos + static_cast<uint32_t>(len);
                if(nlen > m_alloc)
                    grow(nlen - m_len);

                //Replace or insert, we don't care
                Mem::copy(m_data + pos, data, len);
                if(nlen > m_len)
                    m_len = nlen;
            }

            return len;
        }

        int read(uint32_t pos, uint8_t *dst, int len)
        {
            if(pos >= m_len)
                return 0;
            else {
                uint32_t remaining = m_len - pos;
                if(len > static_cast<int>(remaining))
                    len = static_cast<int>(remaining);

                Mem::copy(dst, m_data + pos, len);
                return len;
            }
        }

    private:
        void grow(uint32_t addsz)
        {
            addsz += m_len;
            uint32_t a = m_alloc + (m_alloc >> 1);

            if(a < addsz)
                realloc(addsz);
            else
                realloc(a);
        }

        void realloc(uint32_t newa)
        {
            m_alloc = newa;

            if(m_data == nullptr)
                m_data = m_al.allocate(newa);
            else
                m_data = m_al.reallocate(m_data, m_len, newa);
        }

        Alloc m_al;
        uint8_t *m_data;
        uint32_t m_alloc;
        uint32_t m_len;
    };

    template<class Alloc> class TByteBufInputStream : public InputStream
    {
        friend class TByteBuf<Alloc>;

    public:
        ~TByteBufInputStream() override
        {
        }

        int read(uint8_t *dst, int sz) override
        {
            mDebugAssert(sz >= 0, "cannot read a negative amount of byte");
            int ret = m_ptr->read(m_pos, dst, sz);
            if(ret > 0)
                m_pos += ret;

            return ret;
        }

        uint64_t pos() override
        {
            return static_cast<uint64_t>(m_pos);
        }

        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
        {
            switch(sp) {
            case SeekPos::Beginning:
                mDebugAssert(amount >= 0, "cannot seek backward from beginning");
                if(static_cast<uint32_t>(amount) > m_ptr->size())
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
                    if(m_pos + amnt > m_ptr->size())
                        return false;

                    m_pos += amnt;
                }

                return true;

            case SeekPos::End:
				if(amount > 0)
					return false;

                m_pos = m_ptr->size() - static_cast<uint32_t>(-amount);
                return true;

            default:
                //Should never happen
                return false;
            }
        }

        bool seekSupported() const override
        {
            return true;
        }

        void close() override
        {
        }

    private:
        TByteBufInputStream(TByteBuf<Alloc> *ptr)
        {
            m_ptr = ptr;
            m_pos = 0;
        }

        TByteBufInputStream()
        {
        }

        TByteBufInputStream(const TByteBufInputStream<Alloc> &src)
        {
        }

        TByteBufInputStream(TByteBufInputStream<Alloc> &&src)
        {
        }

        TByteBuf<Alloc> *m_ptr;
        uint32_t m_pos;
    };

    template<class Alloc> class TByteBufOutputStream : public OutputStream
    {
        friend class TByteBuf<Alloc>;

    public:
        ~TByteBufOutputStream() override
        {
            m_ptr->ensureReachable(m_pos);
        }

        int write(const uint8_t *dst, int sz) override
        {
            mDebugAssert(sz >= 0, "cannot write a negative amount of byte");
            int ret = m_ptr->write(m_pos, dst, sz);
            if(ret > 0)
                m_pos += ret;

            return ret;
        }

        uint64_t pos() override
        {
            return static_cast<uint64_t>(m_pos);
        }

        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override
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
                    //Make sure abs(amount) <= m_ptr->size()
                    uint32_t amnt = static_cast<uint32_t>(-amount);
                    if(amnt > m_ptr->size())
                        return false;

                    m_pos = m_ptr->size() - amnt;
                } else
                    m_pos = m_ptr->size() + static_cast<uint32_t>(amount);

                return true;

            default:
                //Should never happen
                return false;
            }
        }

        bool seekSupported() const override
        {
            return true;
        }

        bool flush() override
        {
            m_ptr->ensureReachable(m_pos);
            return true;
        }

        void close() override
        {
            m_ptr->ensureReachable(m_pos);
        }

    private:
        TByteBufOutputStream(TByteBuf<Alloc> *ptr)
        {
            m_ptr = ptr;
            m_pos = 0;
        }

        TByteBufOutputStream()
        {
        }

        TByteBufOutputStream(const TByteBufOutputStream<Alloc> &src)
        {
        }

        TByteBufOutputStream(TByteBufOutputStream<Alloc> &&src)
        {
        }

        TByteBuf<Alloc> *m_ptr;
        uint32_t m_pos;
    };

    template<class Alloc> template<class RefCnt> SharedPtr<InputStream, RefCnt> TByteBuf<Alloc>::inputStream()
    {
        return SharedPtr<InputStream, RefCnt>(new TByteBufInputStream<Alloc>(this));
    }

    template<class Alloc> template<class RefCnt> SharedPtr<OutputStream, RefCnt> TByteBuf<Alloc>::outputStream()
    {
        return SharedPtr<OutputStream, RefCnt>(new TByteBufOutputStream<Alloc>(this));
    }

    typedef TByteBuf<Allocator<uint8_t>> ByteBuf;
    typedef TByteBufInputStream<Allocator<uint8_t>> ByteBufInputStream;
    typedef TByteBufOutputStream<Allocator<uint8_t>> ByteBufOutputStream;
}

