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

#include "mgpcl/NetLogger.h"
#include "mgpcl/Process.h"
#include "mgpcl/File.h"
#include "mgpcl/Time.h"

void m::NetLogger::vlog(LogLevel level, const char *fname, int line, const char *format, VAList *lst)
{
	if(isEnabled(level)) {
		int fnameLen = 0;
		int slashPos = 0;

		while(fname[fnameLen] != 0) {
			if(fname[fnameLen] == '/' || fname[fnameLen] == '\\')
				slashPos = fnameLen + 1;

			fnameLen++;
		}

		String tName(Thread::currentThreadName());
		String shortFName(fname + slashPos, fnameLen - slashPos);
		String content(String::vformat(format, lst));

		Packet pkt(sizeof(uint8_t) + sizeof(uint16_t) + tName.length() + sizeof(uint16_t) + shortFName.length() + sizeof(uint16_t) + sizeof(uint16_t) + content.length());
		pkt << static_cast<uint8_t>(level);
		pkt << tName;
		pkt << shortFName;
		pkt << static_cast<uint16_t>(line);
		pkt << content;

		m_cli.send(pkt.finalize());
	}
}

bool m::NetLogger::connect(const String &ip)
{
	IPv4Address addr;
	if(addr.parse(ip, 1234) != kAFE_NoError)
		return false;

	m_connErr = m_cli.connect(addr);
	for(int i = 0; m_connErr != kSCE_NoError && i < m_attempts; i++) {
		time::sleepMs(1000);
		m_connErr = m_cli.connect(addr);
	}

	return m_connErr == kSCE_NoError;
}

bool m::NetLogger::onPacketReceived(TCPClient *cli)
{
	PacketReader pkt(m_cli.nextPacket());
	uint8_t op;
	uint8_t logLevel;

	if(pkt.size() >= 2 * sizeof(uint8_t)) {
		pkt >> op >> logLevel;

		if(op == 0xEE) {
			//Enable log level
			m_lock.lockFor(RWAction::Writing);
			m_filter |= 1 << static_cast<uint32_t>(logLevel);
			m_lock.releaseFor(RWAction::Writing);
		} else if(op == 0xDD) {
			//Disable log level
			m_lock.lockFor(RWAction::Writing);
			m_filter &= ~(1 << static_cast<uint32_t>(logLevel));
			m_lock.releaseFor(RWAction::Writing);
		} else {
			//Unknown packet... what do we do?
		}
	}

	return false;
}

bool m::NetLogger::tryAutoStartSubprocess(bool cod) const
{
	List<ProcessInfo> processes;
	if(!Process::enumerateProcesses(processes))
		return false;

	for(const ProcessInfo &proc: processes) {
		String cmd(proc.commandName().lower());

#ifdef MGPCL_WIN
		if(cmd.endsWith(".exe"))
			cmd.erase(cmd.length() - 4);
#endif

		if(cmd == "java" || cmd == "javaw") {
			String cmdLine;
			if(!proc.commandLine(cmdLine))
				return false;

			List<String> args;
			parseArgs(cmdLine, args, false);

			for(int i = 0; i < args.size() - 1; i++) {
				if(args[i].toLower() == "-jar") {
					String fname(File(args[i + 1]).fileName().lower());
					if(fname.startsWith("net-logger-") && fname.endsWith(".jar"))
						return true; //Gotcha! Already started :p

					break;
				}
			}
		}
	}

	//So we couldn't find any matching java process. Let's start it up!
	File dir(File::workingDirectory()); //File(".") also works. At least on windows; TODO: Try on linux
	bool keepGoing = true;

	for(int limit = 0; keepGoing && limit < 16; limit++) {
		for(File &child: dir) {
			if(child.isDirectory() && child.fileName() == "net-logger") {
				keepGoing = false;
				break;
			}
		}

		if(keepGoing) {
			dir = dir.parent();

			if(dir.isEmpty())
				return false;
		}
	}

	if(keepGoing)
		return false;

	dir.setPath(dir.path() + "/net-logger/build/libs");
	if(!dir.isDirectory())
		return false;

	for(File &child: dir) {
		if(child.isFile()) {
			String fname(child.fileName().lower());

			if(fname.startsWith("net-logger-") && fname.endsWith(".jar")) //Filter versions
				return startSubprocess(child.path(), cod);
		}
	}

	return false;
}

bool m::NetLogger::startSubprocess(const String &jar, bool cod) const
{
	String java(Process::env("JAVA_HOME"));
	if(java.isEmpty())
		return false;

#ifdef MGPCL_WIN
	java += "\\bin\\javaw.exe";
#else
	java += "/bin/java";
#endif

	File javaExe(java);
	if(!javaExe.isFile())
		return false;

	Process p;
	p.setExecutable(javaExe.path()).pushArg("-jar").pushArg(jar);

	if(cod)
		p.pushArg("--close-on-disconnect");

	return p.start().hasStarted();
}
