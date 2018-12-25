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
#include "IOStream.h"
#include "String.h"
#include "RefCounter.h"
#include "SharedPtr.h"
#include "List.h"

#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termio.h>
#endif

namespace m
{
    class SerialPort;
    class SerialInputStream;
    class SerialOutputStream;

    class SerialHandle
    {
        friend class SerialPort;
        friend class SerialInputStream;
        friend class SerialOutputStream;

    public:
        SerialHandle();

        bool isValid() const
        {
#ifdef MGPCL_WIN
            return m_handle != INVALID_HANDLE_VALUE;
#else
            return m_fd >= 0;
#endif
        }

        bool operator ! () const
        {
#ifdef MGPCL_WIN
            return m_handle == INVALID_HANDLE_VALUE;
#else
            return m_fd < 0;
#endif
        }

        void addRef()
        {
            m_refs.addRef();
        }

        void releaseRef();

        bool isNonBlocking() const
        {
            return m_nbio;
        }

    private:
        AtomicRefCounter m_refs;
        bool m_nbio;

#ifdef MGPCL_WIN
        HANDLE m_handle;
#else
        int m_fd;
#endif
    };

    class SerialInputStream : public InputStream
    {
        M_NON_COPYABLE(SerialInputStream)

    public:
        SerialInputStream(SerialHandle *serial);
        SerialInputStream(SerialInputStream &&src);
        ~SerialInputStream() override;

        int read(uint8_t *dst, int sz) override;
        uint64_t pos() override;
        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override;
        bool seekSupported() const override;
        int available() const;
        void close() override;

        SerialInputStream &operator = (SerialInputStream &&src);

        bool isNonBlocking() const
        {
            return m_serial != nullptr && m_serial->m_nbio;
        }

    private:
        SerialInputStream()
        {
        }

        uint64_t m_pos;
        SerialHandle *m_serial;
    };

    class SerialOutputStream : public OutputStream
    {
        M_NON_COPYABLE(SerialOutputStream)

    public:
        SerialOutputStream(SerialHandle *serial);
        SerialOutputStream(SerialOutputStream &&src);
        ~SerialOutputStream() override;

        int write(const uint8_t *src, int sz) override;
        uint64_t pos() override;
        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override;
        bool seekSupported() const override;
        bool flush() override;
        void close() override;

        SerialOutputStream &operator = (SerialOutputStream &&src);

    private:
        SerialOutputStream()
        {
        }

        uint64_t m_pos;
        SerialHandle *m_serial;
    };

    class SerialPort
    {
        M_NON_COPYABLE(SerialPort)

    public:
        enum AccessFlags
        {
            kAF_Read = 1,
            kAF_Write = 2,
            kAF_ReadWrite = 3 //Read + Write
        };

        enum BaudRate
        {
            kBR_110 = 110,
            kBR_300 = 300,
            kBR_600 = 600,
            kBR_1200 = 1200,
            kBR_2400 = 2400,
            kBR_4800 = 4800,
            kBR_9600 = 9600,
            kBR_19200 = 19200,
            kBR_38400 = 38400,
            kBR_57600 = 57600,
            kBR_115200 = 115200
        };

        enum Parity
        {
            kP_NoParity = 0,
            kP_Odd,
            kP_Even
        };

        enum StopBits
        {
            kSB_One,
            kSB_Two
        };

        SerialPort();
        SerialPort(SerialPort &&sp);
        ~SerialPort();

        bool open(const String &port, int accessFlags = kAF_ReadWrite);
        void setBaudRate(BaudRate br);
        void setStopBits(StopBits sb);
        void setParity(Parity p);
        void setByteSize(uint8_t bs);
        void setArduinoConfig(BaudRate br = kBR_9600);
        bool applyConfig(bool flush = true, int fPause = 2000); //With flush set to true, applyConfig will hang for fPause ms.
        bool setNonBlocking(bool nb = true); //Non blocking mode makes read() return 0 when no data is available
        void close();

        BaudRate baudRate() const;
        StopBits stopBits() const;
        Parity parity() const;
        uint8_t byteSize() const;

        bool isNonBlocking() const
        {
            return m_sh != nullptr && m_sh->m_nbio;
        }

        SerialPort &operator = (SerialPort &&sp);

        bool isOpen() const
        {
            return m_sh != nullptr && m_sh->isValid();
        }

        template<class RefCnt> SharedPtr<SerialInputStream, RefCnt> inputStream()
        {
            if(m_sh == nullptr || !m_sh->isValid())
                return SharedPtr<SerialInputStream, RefCnt>(); //nullptr

            return SharedPtr<SerialInputStream, RefCnt>(new SerialInputStream(m_sh));
        }

        template<class RefCnt> SharedPtr<SerialOutputStream, RefCnt> outputStream()
        {
            if(m_sh == nullptr || !m_sh->isValid())
                return SharedPtr<SerialOutputStream, RefCnt>(); //nullptr

            return SharedPtr<SerialOutputStream, RefCnt>(new SerialOutputStream(m_sh));
        }

#ifdef MGPCL_WIN
        DCB &raw()
        {
            return m_cfg;
        }
#else
        struct termios &raw()
        {
            return m_tty;
        }
#endif

        static bool listDevices(List<String> &dst);

    private:
        SerialHandle *m_sh;

#ifdef MGPCL_WIN
        DCB m_cfg;
        COMMTIMEOUTS m_defTimeouts;
#else
        struct termios m_tty;
#endif
    };
}
