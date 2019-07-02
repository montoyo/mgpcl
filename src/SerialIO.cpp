/* Copyright (C) 2019 BARBOTIN Nicolas
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

#include "mgpcl/SerialIO.h"
#include "mgpcl/Mem.h"

/************************************************************** SerialHandle ***********************************************************************/

m::SerialHandle::SerialHandle() : m_refs(1)
{
#ifdef MGPCL_WIN
    m_handle = INVALID_HANDLE_VALUE;
#else
    m_fd = -1;
#endif
}

void m::SerialHandle::releaseRef()
{
    if(m_refs.releaseRef()) {
#ifdef MGPCL_WIN
        if(m_handle != INVALID_HANDLE_VALUE)
            CloseHandle(m_handle);
#else
        if(m_fd >= 0)
            ::close(m_fd);
#endif

        delete this;
    }
}

/************************************************************** SerialInputStream ***********************************************************************/

m::SerialInputStream::SerialInputStream(SerialHandle *serial)
{
    m_pos = 0;
    m_serial = serial;
    serial->addRef();
}

m::SerialInputStream::SerialInputStream(SerialInputStream &&src)
{
    m_pos = src.m_pos;
    m_serial = src.m_serial;
    src.m_serial = nullptr;
}

m::SerialInputStream::~SerialInputStream()
{
    if(m_serial != nullptr)
        m_serial->releaseRef();
}

int m::SerialInputStream::read(uint8_t *dst, int sz)
{
    mDebugAssert(m_serial != nullptr, "can't read from closed serial port");

#ifdef MGPCL_WIN
    DWORD ret;
    if(ReadFile(m_serial->m_handle, dst, static_cast<DWORD>(sz), &ret, nullptr) == FALSE)
        return -1;

    m_pos += static_cast<uint64_t>(ret);
    return static_cast<int>(ret);
#else
    ssize_t ret = ::read(m_serial->m_fd, dst, static_cast<size_t>(sz));
    if(ret < 0)
        return -1;

    m_pos += static_cast<uint64_t>(ret);
    return static_cast<int>(ret);
#endif
}

uint64_t m::SerialInputStream::pos()
{
    return m_pos;
}

bool m::SerialInputStream::seek(int amount, SeekPos sp)
{
    return false;
}

bool m::SerialInputStream::seekSupported() const
{
    return false;
}

void m::SerialInputStream::close()
{
    if(m_serial != nullptr) {
        m_serial->releaseRef();
        m_serial = nullptr;
    }
}

m::SerialInputStream &m::SerialInputStream::operator = (SerialInputStream &&src)
{
    if(m_serial != nullptr)
        m_serial->releaseRef();

    m_pos = src.m_pos;
    m_serial = src.m_serial;
    src.m_serial = nullptr;
    return *this;
}

int m::SerialInputStream::available() const
{
    if(m_serial == nullptr)
        return -1;

#ifdef MGPCL_WIN
    DWORD noCare;
    COMSTAT commStat;
    if(ClearCommError(m_serial->m_handle, &noCare, &commStat) == FALSE)
        return -1;

    return static_cast<int>(commStat.cbInQue);
#else
    int avail = -1;
    if(ioctl(m_serial->m_fd, FIONREAD, &avail) == -1)
        return -1;

    return avail;
#endif
}

/************************************************************** SerialOutputStream ***********************************************************************/

m::SerialOutputStream::SerialOutputStream(SerialHandle *serial)
{
    m_pos = 0;
    m_serial = serial;
    serial->addRef();
}

m::SerialOutputStream::SerialOutputStream(SerialOutputStream &&src)
{
    m_pos = src.m_pos;
    m_serial = src.m_serial;
    src.m_serial = nullptr;
}

m::SerialOutputStream::~SerialOutputStream()
{
    if(m_serial != nullptr)
        m_serial->releaseRef();
}

int m::SerialOutputStream::write(const uint8_t *dst, int sz)
{
    mDebugAssert(m_serial != nullptr, "can't read from closed serial port");

#ifdef MGPCL_WIN
    DWORD ret;
    if(WriteFile(m_serial->m_handle, dst, static_cast<DWORD>(sz), &ret, nullptr) == FALSE)
        return -1;

    m_pos += static_cast<uint64_t>(ret);
    return static_cast<int>(ret);
#else
    ssize_t ret = ::write(m_serial->m_fd, dst, static_cast<size_t>(sz));
    if(ret < 0)
        return -1;

    m_pos += static_cast<uint64_t>(ret);
    return static_cast<int>(ret);
#endif
}

uint64_t m::SerialOutputStream::pos()
{
    return m_pos;
}

bool m::SerialOutputStream::seek(int amount, SeekPos sp)
{
    return false;
}

bool m::SerialOutputStream::seekSupported() const
{
    return false;
}

bool m::SerialOutputStream::flush()
{
#ifdef MGPCL_WIN
    return FlushFileBuffers(m_serial->m_handle) != FALSE;
#else
    return fsync(m_serial->m_fd) == 0;
#endif
}

void m::SerialOutputStream::close()
{
    if(m_serial != nullptr) {
        m_serial->releaseRef();
        m_serial = nullptr;
    }
}

m::SerialOutputStream & m::SerialOutputStream::operator=(SerialOutputStream &&src)
{
    if(m_serial != nullptr)
        m_serial->releaseRef();

    m_pos = src.m_pos;
    m_serial = src.m_serial;
    src.m_serial = nullptr;
    return *this;
}

/************************************************************** SerialPort ***********************************************************************/

m::SerialPort::SerialPort()
{
    m_sh = nullptr;
}

m::SerialPort::~SerialPort()
{
    if(m_sh != nullptr)
        m_sh->releaseRef();
}

bool m::SerialPort::open(const String &port, int accessFlags)
{
    if(m_sh != nullptr) {
        m_sh->releaseRef();
        m_sh = nullptr;
    }

    if((accessFlags & kAF_ReadWrite) == 0)
        return false;

#ifdef MGPCL_WIN
    if(!port.startsWith("COM", 3))
        return false;

    for(int i = 3; i < port.length(); i++) {
        if(port[i] < '0' || port[i] > '9')
            return false;
    }

    String fname(4 + port.length());
    fname.append("\\\\.\\", 4);
    fname += port;

    DWORD oMode = 0;
    if(accessFlags & kAF_Read)
        oMode |= GENERIC_READ;
    
    if(accessFlags & kAF_Write)
        oMode |= GENERIC_WRITE;

    HANDLE h = CreateFile(fname.raw(), oMode, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(h == INVALID_HANDLE_VALUE)
        return false;

    if(GetCommState(h, &m_cfg) == FALSE) {
        CloseHandle(h);
        return false;
    }

    if(GetCommTimeouts(h, &m_defTimeouts) == FALSE) {
        CloseHandle(h);
        return false;
    }

    m_sh = new SerialHandle;
    m_sh->m_handle = h;
    m_sh->m_nbio = false;
    return true;
#else
    if(!port.startsWith("/dev/tty", 8))
        return false;

    int oMode;
    if((accessFlags & kAF_ReadWrite) == kAF_ReadWrite)
        oMode = O_RDWR;
    else if(accessFlags & kAF_Read)
        oMode = O_RDONLY;
    else
        oMode = O_WRONLY;

    /* TODO: Lack of error reporting here. This is important
     * especially on serial ports because it may throw an
     * 'Access denied' error because of the way linux sucks.
     *
     * O_NDELAY is needed to avoid blocking whatever the state
     * of DCD. It'll disabled just after the open() call.
     */
    int fd = ::open(port.raw(), oMode | O_NOCTTY | O_NDELAY);
    if(fd < 0)
        return false;

    if(fcntl(fd, F_SETFL, 0) == -1) { //Disable non-blocking by default
        ::close(fd);
        return false;
    }

    mem::zero(m_tty);
    if(tcgetattr(fd, &m_tty) != 0) {
        ::close(fd);
        return false;
    }

    m_sh = new SerialHandle;
    m_sh->m_fd = fd;
    m_sh->m_nbio = false;

    //Disable hardware flow control (if supported by the OS)
#ifdef CRTSCTS
    m_tty.c_cflag &= ~CRTSCTS;
#endif
#ifdef CNEW_RTSCTS
    m_tty.c_cflag &= ~CNEW_RTSCTS;
#endif
#ifndef XCASE
#define XCASE 0
#endif

    m_tty.c_cflag |= (CLOCAL | CREAD);
    m_tty.c_lflag &= ~(ISIG | ICANON | XCASE | ECHO | ECHOE);
    m_tty.c_iflag &= ~(IGNBRK | IXON | IXOFF | IXANY | INLCR | ICRNL | IUCLC | PARMRK | ISTRIP);
    m_tty.c_iflag |= INPCK; //Force parity check on. Is that a good thing?
    m_tty.c_oflag &= ~(OPOST | OLCUC | ONLCR | OCRNL | ONLRET | OFILL | NLDLY | CRDLY | TABDLY | BSDLY);
    m_tty.c_oflag &= ~(VTDLY | FFDLY);
    m_tty.c_cc[VMIN] = 0;
    m_tty.c_cc[VTIME] = 5;

    return true;
#endif
}

void m::SerialPort::setBaudRate(BaudRate br)
{
#ifdef MGPCL_WIN
    m_cfg.BaudRate = static_cast<DWORD>(br);
#else
    speed_t spd;
    switch(br) {
    case kBR_110:
        spd = B110;
        break;

    case kBR_300:
        spd = B300;
        break;

    case kBR_600:
        spd = B600;
        break;

    case kBR_1200:
        spd = B1200;
        break;

    case kBR_2400:
        spd = B2400;
        break;

    case kBR_4800:
        spd = B4800;
        break;

    case kBR_9600:
        spd = B9600;
        break;

    case kBR_19200:
        spd = B19200;
        break;

    case kBR_38400:
        spd = B38400;
        break;

    case kBR_57600:
        spd = B57600;
        break;

    case kBR_115200:
        spd = B115200;
        break;

    default:
        spd = B0; //HANG UP!!
        break;
    }

    cfsetispeed(&m_tty, spd);
    cfsetospeed(&m_tty, spd);
#endif
}

void m::SerialPort::setStopBits(StopBits sb)
{
#ifdef MGPCL_WIN
    switch(sb) {
    case kSB_Two:
        m_cfg.StopBits = TWOSTOPBITS;
        break;

    default:
        m_cfg.StopBits = ONESTOPBIT;
        break;
    }
#else
    switch(sb) {
    case kSB_Two:
        m_tty.c_cflag |= CSTOPB;
        break;

    default:
        m_tty.c_cflag &= ~CSTOPB;
        break;
    }
#endif
}

void m::SerialPort::setParity(Parity p)
{
#ifdef MGPCL_WIN
    switch(p) {
    case kP_Odd:
        m_cfg.Parity = ODDPARITY;
        m_cfg.fParity = TRUE;
        break;

    case kP_Even:
        m_cfg.Parity = EVENPARITY;
        m_cfg.fParity = TRUE;
        break;

    default:
        m_cfg.Parity = NOPARITY;
        m_cfg.fParity = FALSE;
        break;
    }
#else
    switch(p) {
    case kP_Odd:
        m_tty.c_cflag |= (PARENB | PARODD);
        break;

    case kP_Even:
        m_tty.c_cflag |= PARENB;
        m_tty.c_cflag &= ~PARODD;
        break;

    default:
        m_tty.c_cflag &= ~(PARENB | PARODD);
        break;
    }
#endif
}

void m::SerialPort::setByteSize(uint8_t bs)
{
#ifdef MGPCL_WIN
    m_cfg.ByteSize = static_cast<BYTE>(bs);
#else
    m_tty.c_cflag &= ~CSIZE;

    switch(bs) {
    case 5:
        m_tty.c_cflag |= CS5;
        break;

    case 6:
        m_tty.c_cflag |= CS6;
        break;

    case 7:
        m_tty.c_cflag |= CS7;
        break;

    case 8:
        m_tty.c_cflag |= CS8;
        break;

    default:
        //Well...
        break;
    }
#endif
}

m::SerialPort::BaudRate m::SerialPort::baudRate() const
{
#ifdef MGPCL_WIN
    return static_cast<BaudRate>(m_cfg.BaudRate);
#else
    speed_t br = cfgetospeed(&m_tty); //Use output speed...

    switch(br) {
    case B110:
        return kBR_110;

    case B300:
        return kBR_300;

    case B600:
        return kBR_600;

    case B1200:
        return kBR_1200;

    case B2400:
        return kBR_2400;

    case B4800:
        return kBR_4800;

    case B9600:
        return kBR_9600;

    case B19200:
        return kBR_19200;

    case B38400:
        return kBR_38400;

    case B57600:
        return kBR_57600;

    case B115200:
        return kBR_115200;

    default:
        return static_cast<BaudRate>(0);
    }
#endif
}

m::SerialPort::StopBits m::SerialPort::stopBits() const
{
#ifdef MGPCL_WIN
    switch(m_cfg.StopBits) {
    case TWOSTOPBITS:
        return kSB_Two;

    default:
        return kSB_One;
    }
#else
    if(m_tty.c_cflag & CSTOPB)
        return kSB_Two;

    return kSB_One;
#endif
}

m::SerialPort::Parity m::SerialPort::parity() const
{
#ifdef MGPCL_WIN
    switch(m_cfg.Parity) {
    case ODDPARITY:
        return kP_Odd;

    case EVENPARITY:
        return kP_Even;

    default:
        return kP_NoParity;
    }
#else
    if(m_tty.c_cflag & PARENB == 0)
        return kP_NoParity;

    return (m_tty.c_cflag & PARODD) == 0 ? kP_Even : kP_Odd;
#endif
}

uint8_t m::SerialPort::byteSize() const
{
#ifdef MGPCL_WIN
    return static_cast<uint8_t>(m_cfg.ByteSize);
#else
    tcflag_t bs = m_tty.c_cflag & CSIZE;

    if(bs & CS5)
        return 5;
    else if(bs & CS6)
        return 6;
    else if(bs & CS7)
        return 7;
    else if(bs & CS8)
        return 8;

    return 0;
#endif
}

void m::SerialPort::setArduinoConfig(BaudRate br)
{
#ifdef MGPCL_WIN
    m_cfg.fRtsControl = RTS_CONTROL_DISABLE;
    m_cfg.fParity = FALSE;
    m_cfg.BaudRate = static_cast<DWORD>(br);
    m_cfg.StopBits = ONESTOPBIT;
    m_cfg.Parity = NOPARITY;
    m_cfg.ByteSize = 8;
#else
    m_tty.c_cflag &= ~(CSTOPB | PARENB | PARODD | CSIZE);
    m_tty.c_cflag |= CS8;
    setBaudRate(br);
#endif
}

bool m::SerialPort::applyConfig(bool flush, int fPause)
{
    mAssert(m_sh != nullptr && m_sh->isValid(), "didn't open serial port!");

#ifdef MGPCL_WIN
    if(SetCommState(m_sh->m_handle, &m_cfg) == FALSE)
        return false;

    SetupComm(m_sh->m_handle, MGPCL_WIN_SERIAL_BUF_SZ, MGPCL_WIN_SERIAL_BUF_SZ);

    if(flush) {
        Sleep(static_cast<DWORD>(fPause));
        PurgeComm(m_sh->m_handle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
    }

    return true;
#else
    if(tcsetattr(m_sh->m_fd, TCSANOW, &m_tty) != 0)
        return false;

    if(flush) {
        usleep(static_cast<unsigned int>(fPause * 1000));
        tcflush(m_sh->m_fd, TCIFLUSH);
    }

    return true;
#endif
}

void m::SerialPort::close()
{
    if(m_sh != nullptr) {
        m_sh->releaseRef();
        m_sh = nullptr;
    }
}

m::SerialPort::SerialPort(SerialPort &&sp)
{
    m_sh = sp.m_sh;
    sp.m_sh = nullptr;

#ifdef MGPCL_WIN
    m_cfg = sp.m_cfg;
#else
    m_tty = sp.m_tty;
#endif
}

m::SerialPort &m::SerialPort::operator = (SerialPort &&sp)
{
    if(m_sh != nullptr)
        m_sh->releaseRef();

    m_sh = sp.m_sh;
    sp.m_sh = nullptr;

#ifdef MGPCL_WIN
    m_cfg = sp.m_cfg;
#else
    m_tty = sp.m_tty;
#endif

    return *this;
}

bool m::SerialPort::setNonBlocking(bool nb)
{
    if(m_sh == nullptr)
        return false;

#ifdef MGPCL_WIN
    if(nb) {
        COMMTIMEOUTS cto = m_defTimeouts;
        cto.ReadIntervalTimeout = MAXDWORD;
        cto.ReadTotalTimeoutMultiplier = 0;
        cto.ReadTotalTimeoutConstant = 0;

        if(SetCommTimeouts(m_sh->m_handle, &cto) == FALSE)
            return false;
    } else if(SetCommTimeouts(m_sh->m_handle, &m_defTimeouts) == FALSE)
        return false;

    m_sh->m_nbio = nb;
    return true;
#else
    if(fcntl(m_sh->m_fd, F_SETFL, nb ? FNDELAY : 0) == -1)
        return false;

    m_sh->m_nbio = nb;
    return true;
#endif
}

#ifdef MGPCL_WIN
#include "mgpcl/WinWMI.h"

bool m::SerialPort::listDevices(List<String> &dst)
{
    if(!wmi::acquire()) {
        wmi::release();
        return false;
    }

    WMIResult *data = wmi::query("SELECT DeviceID FROM Win32_SerialPort");
    if(data == nullptr) {
        wmi::release();
        return false;
    }

    while(data->next()) {
        String dev(data->getString(L"DeviceID"));

        if(dev.startsWith("COM"))
            dst.add(dev);
    }

    data->releaseRef();
    wmi::release();
    return true;
}

#else
#include <dirent.h>
#include <cstring>

bool m::SerialPort::listDevices(List<String> &dst)
{
    DIR *ttys = opendir("/sys/class/tty");
    if(ttys == nullptr)
        return false;

    struct dirent *entry;
    while((entry = readdir(ttys)) != nullptr) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            String fname(15 + 8 + 14);
            fname.append("/sys/class/tty/", 15);
            fname += entry->d_name;
            fname.append("/device/driver", 14);

            if(access(fname.raw(), F_OK) == 0) {
                String dev(5 + 8);
                dev.append("/dev/", 5);
                dev += entry->d_name;

                dst.add(dev);
            }
        }
    }

    closedir(ttys);
    return true;
}

#endif
