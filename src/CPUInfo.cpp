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

#include "mgpcl/CPUInfo.h"

#ifdef MGPCL_WIN
#include "mgpcl/WinWMI.h"

m::CPUInfo m::CPUInfo::fetch()
{
	if(!wmi::acquire()) {
		CPUInfo err(wmi::lastError());
		wmi::release();
		return err;
	}

	WMIResult *obj = wmi::query("SELECT * FROM Win32_Processor");
	if(obj == nullptr) {
		CPUInfo err(wmi::lastError());
		wmi::release();
		return err;
	}

	if(!obj->next()) {
		CPUInfo err(wmi::lastError().isEmpty() ? "Got no data" : wmi::lastError());
		wmi::release();
		return err;
	}

	CPUInfo ret;
	ret.m_name = obj->getString(L"Name");
	ret.m_vendor = obj->getString(L"Manufacturer");
	ret.m_cores = obj->getUInt32(L"NumberOfCores");
	ret.m_maxFreq = obj->getUInt32(L"MaxClockSpeed");
	
	obj->releaseRef();
	wmi::release();
	return ret;
}

#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

class FBuf
{
public:
	FBuf(int fd)
	{
		m_fd = fd;
		m_pos = 0;
		m_sz = 0;
		m_err = false;
	}

	char next()
	{
		if(m_pos >= m_sz) {
			//Refill
			ssize_t rd = read(m_fd, m_tmp, 256);
			if(rd < 0) {
				m_err = true;
				return 0;
			} else if(rd == 0)
				return 0;

			m_pos = 0;
			m_sz = static_cast<int>(rd);
		}

		return m_tmp[m_pos++];
	}

	bool errored() const
	{
		return m_err;
	}

private:
	int m_fd;
	int m_pos;
	int m_sz;
	char m_tmp[256];
	bool m_err;
};

static bool readKeyVal(FBuf &buf, m::String &key, m::String &val)
{
	bool readingKey = true;
	char c = buf.next();

	while(c != 0) {
		if(readingKey) {
			if(c == '\n')
				return false; //Consider it's EOF, read only one CPU
			else if(c == ':')
				readingKey = false;
			else
				key += c;
		} else {
			if(c == '\n')
				return true;
			else
				val += c;
		}

		c = buf.next();
	}

	return false;
}

m::CPUInfo m::CPUInfo::fetch()
{
	int fd = open("/proc/cpuinfo", O_RDONLY);
	if(fd < 0)
		return CPUInfo("couldn't open /proc/cpuinfo");

	FBuf buf(fd);
	String rawKey, rawVal;
	CPUInfo ret;
	uint8_t mask = 0;

	while(readKeyVal(buf, rawKey, rawVal) && mask != 15) {
		String key(rawKey.trimmed());

		if(key.equalsIgnoreCase("model name")) {
			ret.m_name = rawVal.trimmed();
			mask |= 1;
		} else if(key.equalsIgnoreCase("vendor_id")) {
			ret.m_vendor = rawVal.trimmed();
			mask |= 2;
		} else if(key.equalsIgnoreCase("cpu cores")) {
			ret.m_cores = rawVal.trimmed().toUInteger();
			mask |= 4;
		} else if(key.equalsIgnoreCase("cpu MHz")) {
			int pos = rawVal.indexOf('.'); //Remove comma

			if(pos >= 0)
				ret.m_maxFreq = rawVal.substr(0, pos).trimmedLeft().toUInteger();
			else
				ret.m_maxFreq = rawVal.trimmed().toUInteger();

			mask |= 8;
		}

		rawKey.cleanup();
		rawVal.cleanup();
	}

	close(fd);
	if(buf.errored()) {
		ret.m_valid = false;
		ret.m_error = "couldn't read /proc/cpuinfo";
	}

	return ret;
}

#endif
