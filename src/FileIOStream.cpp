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

#include "mgpcl/FileIOStream.h"
#include "mgpcl/Assert.h"

#ifdef MGPCL_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

m::FileInputStream::FileInputStream()
{
#ifdef MGPCL_WIN
    m_file = INVALID_HANDLE_VALUE;
#else
    m_file = -1;
#endif
}

m::FileInputStream::FileInputStream(const String &fname)
{
#ifdef MGPCL_WIN
    m_file = INVALID_HANDLE_VALUE;
#else
    m_file = -1;
#endif

    open(fname);
}

m::FileInputStream::~FileInputStream()
{
#ifdef MGPCL_WIN
    if(m_file != INVALID_HANDLE_VALUE)
        CloseHandle(m_file);
#else
    if(m_file != -1)
        ::close(m_file);
#endif
}

m::FileInputStream::OpenError m::FileInputStream::open(const String &fname)
{
#ifdef MGPCL_WIN
    if(m_file != INVALID_HANDLE_VALUE)
        CloseHandle(m_file);

    if(fname.length() > MAX_PATH) {
        wchar_t *tmp = new wchar_t[fname.length() + 5];
        size_t sz;

        mbstowcs_s(&sz, tmp, 5, "\\\\?\\", _TRUNCATE);
        mbstowcs_s(&sz, tmp + 4, fname.length() + 1, fname.raw(), _TRUNCATE);
        m_file = CreateFileW(tmp, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        delete[] tmp;
    } else
        m_file = CreateFileA(fname.raw(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if(m_file != INVALID_HANDLE_VALUE)
        return kOE_Success;
    else if(GetLastError() == ERROR_FILE_NOT_FOUND)
        return kOE_FileNotFound;
    else
        return kOE_Unknown;
#else
    if(m_file != -1)
        ::close(m_file);

    m_file = ::open(fname.raw(), O_RDONLY);
    if(m_file != -1)
        return kOE_Success;
    else if(errno == ENOENT)
        return kOE_FileNotFound;
    else
        return kOE_Unknown;
#endif
}

int m::FileInputStream::read(uint8_t *dst, int sz)
{
#ifdef MGPCL_WIN
    mDebugAssert(m_file != INVALID_HANDLE_VALUE, "file was not opened");
    mDebugAssert(sz >= 0, "cannot read a negative amount of bytes");

    DWORD ret;
    if(ReadFile(m_file, dst, static_cast<DWORD>(sz), &ret, nullptr) == FALSE)
        return -1;
    else
        return static_cast<int>(ret); //ReadFile() already sets ret to zero if EOF was reached.
#else
    mDebugAssert(m_file != -1, "file was not opened");
    mDebugAssert(sz >= 0, "cannot read a negative amount of bytes");

    return static_cast<int>(::read(m_file, dst, static_cast<size_t>(sz)));
#endif
}

uint64_t m::FileInputStream::pos()
{
#ifdef MGPCL_WIN
    mDebugAssert(m_file != INVALID_HANDLE_VALUE, "file was not opened");

    LONG h = 0;
    LONG l = SetFilePointer(m_file, 0, &h, FILE_CURRENT);

    return (static_cast<uint64_t>(h) << 32) | static_cast<uint64_t>(l);
#else
    mDebugAssert(m_file != -1, "file was not opened");
    return static_cast<uint64_t>(lseek(m_file, 0, SEEK_CUR));
#endif
}

bool m::FileInputStream::seek(int amount, SeekPos sp)
{
#ifdef MGPCL_WIN
    mDebugAssert(m_file != INVALID_HANDLE_VALUE, "file was not opened");

    DWORD mode;
    switch(sp) {
    case SeekPos::Beginning:
        mDebugAssert(amount >= 0, "cannot seek backward from beginning");
        mode = FILE_BEGIN;
        break;

    case SeekPos::Relative:
        mode = FILE_CURRENT;
        break;

    case SeekPos::End:
        mDebugAssert(amount < 0, "cannot seek forward from the end");
        mode = FILE_END;
        break;

    default:
        //Should never happen.
        return false;
    }

    return SetFilePointer(m_file, amount, nullptr, mode) != INVALID_SET_FILE_POINTER;
#else
    mDebugAssert(m_file != -1, "file was not opened");
    int mode;

    switch(sp) {
    case SeekPos::Beginning:
        mDebugAssert(amount >= 0, "cannot seek backward from beginning");
        mode = SEEK_SET;
        break;

    case SeekPos::Relative:
        mode = SEEK_CUR;
        break;

    case SeekPos::End:
        mDebugAssert(amount < 0, "cannot seek forward from the end");
        mode = SEEK_END;
        break;

    default:
        //Should never happen.
        return false;
    }

    return lseek(m_file, static_cast<off_t>(amount), mode) != -1;
#endif
}

void m::FileInputStream::close()
{
#ifdef MGPCL_WIN
    if(m_file != INVALID_HANDLE_VALUE) {
        CloseHandle(m_file);
        m_file = INVALID_HANDLE_VALUE;
    }
#else
    if(m_file != -1) {
        ::close(m_file);
        m_file = -1;
    }
#endif
}

//*******************************************************************  FILE OUTPUT STREAM *******************************************************************

m::FileOutputStream::FileOutputStream()
{
#ifdef MGPCL_WIN
    m_file = INVALID_HANDLE_VALUE;
#else
    m_file = -1;
#endif
}

m::FileOutputStream::FileOutputStream(const String &fname, OpenMode m)
{
#ifdef MGPCL_WIN
    m_file = INVALID_HANDLE_VALUE;
#else
    m_file = -1;
#endif

    open(fname, m);
}

m::FileOutputStream::~FileOutputStream()
{
#ifdef MGPCL_WIN
    if(m_file != INVALID_HANDLE_VALUE)
        CloseHandle(m_file);
#else
    if(m_file != -1)
        ::close(m_file);
#endif
}

bool m::FileOutputStream::open(const String &fname, OpenMode mode)
{
#ifdef MGPCL_WIN
    if(m_file != INVALID_HANDLE_VALUE)
        CloseHandle(m_file);

    DWORD disposition;
    if(mode == kOM_Truncate)
        disposition = CREATE_ALWAYS;
    else
        disposition = OPEN_ALWAYS;

    if(fname.length() > MAX_PATH) {
        wchar_t *tmp = new wchar_t[fname.length() + 5];
        size_t sz;

        mbstowcs_s(&sz, tmp, 5, "\\\\?\\", _TRUNCATE);
        mbstowcs_s(&sz, tmp + 4, fname.length() + 1, fname.raw(), _TRUNCATE);
        m_file = CreateFileW(tmp, GENERIC_WRITE, FILE_SHARE_READ, nullptr, disposition, FILE_ATTRIBUTE_NORMAL, nullptr);
        delete[] tmp;
    } else
        m_file = CreateFileA(fname.raw(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, disposition, FILE_ATTRIBUTE_NORMAL, nullptr);

    if(m_file == INVALID_HANDLE_VALUE)
        return false;
    else {
        if(mode == kOM_AtEnd) {
            if(SetFilePointer(m_file, 0, nullptr, FILE_END) == INVALID_SET_FILE_POINTER) {
                CloseHandle(m_file);
                return false;
            }
        }

        return true;
    }
#else
    if(m_file != -1)
        ::close(m_file);

    int flags = O_WRONLY | O_CREAT;
    if(mode == kOM_Truncate)
        flags |= O_TRUNC;

    m_file = ::open(fname.raw(), flags, 0644);
    if(m_file == -1)
        return false;
    else {
        if(mode == kOM_AtEnd) {
            if(lseek(m_file, 0, SEEK_END) == -1) {
                ::close(m_file);
                return false;
            }
        }

        return true;
    }
#endif
}

int m::FileOutputStream::write(const uint8_t *src, int sz)
{
#ifdef MGPCL_WIN
    mDebugAssert(m_file != INVALID_HANDLE_VALUE, "file was not opened");
    mDebugAssert(sz >= 0, "cannot write a negative amount of bytes");

    DWORD written;
    if(WriteFile(m_file, src, static_cast<DWORD>(sz), &written, nullptr) == FALSE)
        return -1;
    else
        return static_cast<int>(written);
#else
    mDebugAssert(m_file != -1, "file was not opened");
    mDebugAssert(sz >= 0, "cannot write a negative amount of bytes");

    return static_cast<int>(::write(m_file, src, static_cast<size_t>(sz)));
#endif
}

uint64_t m::FileOutputStream::pos()
{
#ifdef MGPCL_WIN
    mDebugAssert(m_file != INVALID_HANDLE_VALUE, "file was not opened");

    LONG h = 0;
    LONG l = SetFilePointer(m_file, 0, &h, FILE_CURRENT);

    return (static_cast<uint64_t>(h) << 32) | static_cast<uint64_t>(l);
#else
    mDebugAssert(m_file != -1, "file was not opened");
    return static_cast<uint64_t>(lseek(m_file, 0, SEEK_CUR));
#endif
}

bool m::FileOutputStream::seek(int amount, SeekPos sp)
{
#ifdef MGPCL_WIN
    mDebugAssert(m_file != INVALID_HANDLE_VALUE, "file was not opened");

    DWORD mode;
    switch(sp) {
    case SeekPos::Beginning:
        mDebugAssert(amount >= 0, "cannot seek backward from beginning");
        mode = FILE_BEGIN;
        break;

    case SeekPos::Relative:
        mode = FILE_CURRENT;
        break;

    case SeekPos::End:
        mDebugAssert(amount < 0, "cannot seek forward from the end");
        mode = FILE_END;
        break;

    default:
        //Should never happen.
        return false;
    }

    return SetFilePointer(m_file, amount, nullptr, mode) != INVALID_SET_FILE_POINTER;
#else
    mDebugAssert(m_file != -1, "file was not opened");
    int mode;

    switch(sp) {
    case SeekPos::Beginning:
        mDebugAssert(amount >= 0, "cannot seek backward from beginning");
        mode = SEEK_SET;
        break;

    case SeekPos::Relative:
        mode = SEEK_CUR;
        break;

    case SeekPos::End:
        mDebugAssert(amount < 0, "cannot seek forward from the end");
        mode = SEEK_END;
        break;

    default:
        //Should never happen.
        return false;
    }

    return lseek(m_file, static_cast<off_t>(amount), mode) != -1;
#endif
}

bool m::FileOutputStream::flush()
{
#ifdef MGPCL_WIN
    mDebugAssert(m_file != INVALID_HANDLE_VALUE, "file was not opened");
    return FlushFileBuffers(m_file) == TRUE;
#else
    mDebugAssert(m_file != -1, "file was not opened");
    return fsync(m_file) == 0;
#endif
}

void m::FileOutputStream::close()
{
#ifdef MGPCL_WIN
    if(m_file != INVALID_HANDLE_VALUE) {
        CloseHandle(m_file);
        m_file = INVALID_HANDLE_VALUE;
    }
#else
    if(m_file != -1) {
        ::close(m_file);
        m_file = -1;
    }
#endif
}
