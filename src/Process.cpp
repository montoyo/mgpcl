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

#include "mgpcl/Process.h"

#ifdef MGPCL_WIN
#include "mgpcl/WinCmdLine.h"
#include <TlHelp32.h>
#else
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif

/************************************************************** ProcessInfo ***********************************************************************/

bool m::ProcessInfo::commandLine(String &dst) const
{
#ifdef MGPCL_WIN
    if(!win::initIfNeeded())
        return false;

    HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_pid);
    if(proc == nullptr)
        return false;

    String fullCmdline(16);
    bool ret = win::getProcessCommandLine(proc, fullCmdline);
    CloseHandle(proc);

    if(ret) {
        dst.cleanup();
        dst += fullCmdline;
        return true;
    }

    return false;
#else
    String clPath(16);
    clPath.append("/proc/", 6);
    clPath += String::fromUInteger(m_pid);
    clPath.append("/cmdline", 8);

    int fd = open(clPath.raw(), O_RDONLY);
    if(fd < 0)
        return false;

    char buf[128];
    dst.cleanup();

    while(true) {
        ssize_t amount = read(fd, buf, 128);
        if(amount < 0) {
            close(fd);
            return false;
        } else if(amount == 0)
            break;

        dst.append(buf, static_cast<int>(amount));
    }

    close(fd);
    return true;
#endif
}

/************************************************************** ProcessPipe ***********************************************************************/

m::ProcessPipe::ProcessPipe()
{
    m_dir = kPPD_None;

#ifdef MGPCL_WIN
    m_rd = INVALID_HANDLE_VALUE;
    m_wr = INVALID_HANDLE_VALUE;
#else
    m_fds[0] = -1;
    m_fds[1] = -1;
#endif
}

bool m::ProcessPipe::create(PipeDirection dir)
{
#ifdef MGPCL_WIN
    if(m_rd != INVALID_HANDLE_VALUE) {
        CloseHandle(m_rd);
        m_rd = INVALID_HANDLE_VALUE;
    }

    if(m_wr != INVALID_HANDLE_VALUE) {
        CloseHandle(m_wr);
        m_wr = INVALID_HANDLE_VALUE;
    }

    m_dir = dir;
    if(dir == kPPD_None)
        return true;

    SECURITY_ATTRIBUTES attrs;
    attrs.nLength = sizeof(SECURITY_ATTRIBUTES);
    attrs.lpSecurityDescriptor = nullptr;
    attrs.bInheritHandle = TRUE;

    if(CreatePipe(&m_rd, &m_wr, &attrs, 0) == FALSE)
        return false;

    switch(dir) {
    case kPPD_ChildInput:
        //Child can't write to stdin, disable inheritance
        return SetHandleInformation(m_wr, HANDLE_FLAG_INHERIT, 0) != FALSE;

    case kPPD_ChildOutput:
        //Child can't read from stdout/stderr, disable inheriance
        return SetHandleInformation(m_rd, HANDLE_FLAG_INHERIT, 0) != FALSE;

    default:
        //Unknown pipe direction...
        return false;
    }
#else
    if(m_fds[0] >= 0) {
        close(m_fds[0]);
        m_fds[0] = -1;
    }

    if(m_fds[1] >= 0) {
        close(m_fds[1]);
        m_fds[1] = -1;
    }

    m_dir = dir;
    if(dir == kPPD_None)
        return true;

    return pipe(m_fds) == 0;
#endif
}

void m::ProcessPipe::closeChildPipe()
{
#ifdef MGPCL_WIN
    HANDLE *h;

    switch(m_dir) {
    case kPPD_ChildInput:
        h = &m_rd;
        break;

    case kPPD_ChildOutput:
        h = &m_wr;
        break;

    default:
        return;
    }

    if(*h != INVALID_HANDLE_VALUE) {
        CloseHandle(*h);
        *h = INVALID_HANDLE_VALUE;
    }
#else
    int *fd;

    switch(m_dir) {
    case kPPD_ChildInput:
        fd = &m_fds[0];
        break;

    case kPPD_ChildOutput:
        fd = &m_fds[1];
        break;

    default:
        return;
    }

    if(*fd >= 0) {
        close(*fd);
        *fd = -1;
    }
#endif
}

void m::ProcessPipe::destroy()
{
#ifdef MGPCL_WIN
    if(m_rd != INVALID_HANDLE_VALUE) {
        CloseHandle(m_rd);
        m_rd = INVALID_HANDLE_VALUE;
    }

    if(m_wr != INVALID_HANDLE_VALUE) {
        CloseHandle(m_wr);
        m_wr = INVALID_HANDLE_VALUE;
    }
#else
    if(m_fds[0] >= 0) {
        close(m_fds[0]);
        m_fds[0] = -1;
    }

    if(m_fds[1] >= 0) {
        close(m_fds[1]);
        m_fds[1] = -1;
    }
#endif
}

/************************************************************** ProcessHandles ***********************************************************************/

m::ProcessPipes::ProcessPipes() : m_refs(1)
{
}

bool m::ProcessPipes::createPipes()
{
    if(!m_pipes[kPPI_StdIn].create(ProcessPipe::kPPD_ChildInput))
        return false;

    if(!m_pipes[kPPI_StdOut].create(ProcessPipe::kPPD_ChildOutput))
        return false;

    return m_pipes[kPPI_StdErr].create(ProcessPipe::kPPD_ChildOutput);
}

void m::ProcessPipes::closeChildPipes()
{
    for(int i = 0; i < kPPI_Count; i++)
        m_pipes[i].closeChildPipe();
}

void m::ProcessPipes::addRef()
{
    m_refs.increment();
}

void m::ProcessPipes::releaseRef()
{
    if(m_refs.decrement()) {
        for(int i = 0; i < kPPI_Count; i++)
            m_pipes[i].destroy();

        delete this;
    }
}

/************************************************************** PipeInputStream ***********************************************************************/

m::PipeInputStream::PipeInputStream(ProcessPipes *handles, ProcessPipeID pid)
{
    m_handles = handles;
    m_pipe = pid;
    m_pos = 0;
    m_handles->addRef();
}

m::PipeInputStream::PipeInputStream(PipeInputStream &&src)
{
    m_handles = src.m_handles;
    m_pipe = src.m_pipe;
    m_pos = src.m_pos;
    src.m_handles = nullptr;
}

m::PipeInputStream::~PipeInputStream()
{
    if(m_handles != nullptr)
        m_handles->releaseRef();
}

int m::PipeInputStream::read(uint8_t *dst, int sz)
{
    mDebugAssert(m_handles != nullptr, "can't read from closed pipe");

#ifdef MGPCL_WIN
    DWORD ret;
    if(ReadFile((*m_handles)[m_pipe].m_rd, dst, static_cast<DWORD>(sz), &ret, nullptr) == FALSE)
        return GetLastError() == ERROR_BROKEN_PIPE ? 0 : -1;

    m_pos += static_cast<uint64_t>(ret);
    return static_cast<int>(ret);
#else
    ssize_t ret = ::read((*m_handles)[m_pipe].m_fds[0], dst, static_cast<size_t>(sz));
    if(ret < 0)
        return -1;

    m_pos += static_cast<uint64_t>(ret);
    return static_cast<int>(ret);
#endif
}

uint64_t m::PipeInputStream::pos()
{
    return m_pos;
}

bool m::PipeInputStream::seek(int amount, SeekPos sp)
{
    return false;
}

bool m::PipeInputStream::seekSupported() const
{
    return false;
}

void m::PipeInputStream::close()
{
    m_handles->releaseRef();
    m_handles = nullptr;
}

m::PipeInputStream &m::PipeInputStream::operator = (PipeInputStream &&src)
{
    if(m_handles != nullptr)
        m_handles->releaseRef();

    m_handles = src.m_handles;
    m_pipe = src.m_pipe;
    m_pos = src.m_pos;
    src.m_handles = nullptr;
    return *this;
}

/************************************************************** PipeOutputStream ***********************************************************************/

m::PipeOutputStream::PipeOutputStream(ProcessPipes *handles, ProcessPipeID pid)
{
    m_handles = handles;
    m_pipe = pid;
    m_pos = 0;
    m_handles->addRef();
}

m::PipeOutputStream::PipeOutputStream(PipeOutputStream &&src)
{
    m_handles = src.m_handles;
    m_pipe = src.m_pipe;
    m_pos = src.m_pos;
    src.m_handles = nullptr;
}

m::PipeOutputStream::~PipeOutputStream()
{
    if(m_handles != nullptr)
        m_handles->releaseRef();
}

int m::PipeOutputStream::write(const uint8_t *dst, int sz)
{
    mDebugAssert(m_handles != nullptr, "can't write to closed pipe");

#ifdef MGPCL_WIN
    DWORD ret;
    if(WriteFile((*m_handles)[m_pipe].m_wr, dst, static_cast<DWORD>(sz), &ret, nullptr) == FALSE)
        return GetLastError() == ERROR_BROKEN_PIPE ? 0 : -1;

    m_pos += static_cast<uint64_t>(ret);
    return static_cast<int>(ret);
#else
    ssize_t ret = ::write((*m_handles)[m_pipe].m_fds[1], dst, static_cast<size_t>(sz));
    if(ret < 0)
        return -1;

    m_pos += static_cast<uint64_t>(ret);
    return static_cast<int>(ret);
#endif
}

uint64_t m::PipeOutputStream::pos()
{
    return m_pos;
}

bool m::PipeOutputStream::seek(int amount, SeekPos sp)
{
    return false;
}

bool m::PipeOutputStream::seekSupported() const
{
    return false;
}

bool m::PipeOutputStream::flush()
{
    return true;
}

void m::PipeOutputStream::close()
{
    m_handles->releaseRef();
    m_handles = nullptr;
}

m::PipeOutputStream &m::PipeOutputStream::operator = (PipeOutputStream &&src)
{
    if(m_handles != nullptr)
        m_handles->releaseRef();

    m_handles = src.m_handles;
    m_pipe = src.m_pipe;
    m_pos = src.m_pos;
    src.m_handles = nullptr;
    return *this;
}

/************************************************************** Process ***********************************************************************/

m::Process::Process()
{
    m_pid = 0;
    m_redirSTD = false;
    m_handles = new ProcessPipes;

#ifdef MGPCL_WIN
    m_process = INVALID_HANDLE_VALUE;
#else
    m_started = false;
    m_finished = false;
    m_retCode = -1;
#endif
}

m::Process::~Process()
{
#ifdef MGPCL_WIN
    if(m_process != INVALID_HANDLE_VALUE)
        CloseHandle(m_process);
#endif

    m_handles->releaseRef();
}

m::Process &m::Process::start()
{
#ifdef MGPCL_WIN
    //Command line
    String cmdLine(m_exe.length() + 2);
    if(m_exe.indexOfAnyOf(" \t\r\n", 0, 4) >= 0) {
        cmdLine += '\"';
        cmdLine += m_exe;
        cmdLine += '\"';
    } else
        cmdLine += m_exe;

    for(const String &a: m_args) {
        //This could be faster really...
        if(a.indexOfAnyOf("\"\\", 0, 2) >= 0) {
            cmdLine.append(" \"", 2);

            //Escape string!
            for(int i = 0; i < a.length(); i++) {
                if(a[i] == '\"')
                    cmdLine.append("\\\"", 2);
                else if(a[i] == '\\')
                    cmdLine.append("\\\\", 2);
                else
                    cmdLine += a[i];
            }

            cmdLine += '\"';
        } else if(a.indexOfAnyOf(" \t\r\n", 0, 4) >= 0) {
            cmdLine.append(" \"", 2);
            cmdLine += a;
            cmdLine += '\"';
        } else {
            cmdLine += ' ';
            cmdLine += a;
        }
    }

    //Env
    String env(m_env.size() * 4 + 1);
    bool noEnv = true;

    for(EnvMap::Pair &it: m_env) {
        if(!it.value.isEmpty()) {
            env += it.key;
            env += '=';
            env += it.value;
            env += '\0';

            if(noEnv)
                noEnv = false;
        }
    }

    env += '\0';

    //STD I/O redirection
    STARTUPINFO si;
    Mem::zero(si);
    si.cb = sizeof(STARTUPINFO);

    if(m_redirSTD) {
        if(!m_handles->createPipes())
            return *this;

        si.hStdInput = (*m_handles)[kPPI_StdIn].m_rd;
        si.hStdOutput = (*m_handles)[kPPI_StdOut].m_wr;
        si.hStdError = (*m_handles)[kPPI_StdErr].m_wr;
        si.dwFlags |= STARTF_USESTDHANDLES;
    }

    PROCESS_INFORMATION pi;
    if(CreateProcess(m_exe.raw(), cmdLine.begin(), nullptr, nullptr, m_redirSTD ? TRUE : FALSE, 0, noEnv ? nullptr : env.begin(), m_workingDir.isEmpty() ? nullptr : m_workingDir.raw(), &si, &pi) == FALSE)
        return *this;

    if(m_redirSTD)
        m_handles->closeChildPipes();

    m_pid = pi.dwProcessId;
    m_process = pi.hProcess;
    CloseHandle(pi.hThread); //We don't need that!
#else
    int nullFd = -1;
    if(m_redirSTD) {
        if(!m_handles->createPipes())
            return *this;
    } else {
        nullFd = open("/dev/null", O_RDWR);

        if(nullFd < 0)
            return *this;
    }

    //Build a list of arguments
    //Pointers should remain valid as long as we don't
    //play with m_args, m_args' contents, m_exe and
    //m_workingDir, which should NOT happen. Theoretically.
    const char *wdir  = m_workingDir.isEmpty() ? nullptr : m_workingDir.raw();
    const char **args = new const char*[m_args.size() + 2];
    const char **env  = nullptr;

    for(int i = 0; i < m_args.size(); i++)
        args[i + 1] = m_args[i].raw();

    args[0]                 = m_exe.raw(); //By convention, args[0] is the process path
    args[m_args.size() + 1] = nullptr;     //Argument list must end with NULL

    List<String> envl(m_env.size());
    for(EnvMap::Pair &it: m_env) {
        if(!it.value.isEmpty())
            envl.add(String(it.key + '=' + it.value));
    }

    if(!envl.isEmpty()) {
        env = new const char*[envl.size() + 1];
        for(int i = 0; i < envl.size(); i++)
            env[i] = envl[i].raw();

        env[envl.size()] = nullptr;
    }

    pid_t pid = fork();
    if(pid < 0) { //Failed
        if(nullFd >= 0)
            close(nullFd);

        return *this;
    }

    if(pid == 0) {
        //This is the child process
        //We avoid any writing operation in the memory
        //Because the child process uses C.O.W.
        if(wdir != nullptr && chdir(wdir) != 0) {
            //chdir failed, self-destruction
            _exit(-1);
        }

        //Also, redirect stdio
        if(m_redirSTD) {
            dup2((*m_handles)[kPPI_StdIn].m_fds[0], STDIN_FILENO);
            dup2((*m_handles)[kPPI_StdOut].m_fds[1], STDOUT_FILENO);
            dup2((*m_handles)[kPPI_StdErr].m_fds[1], STDERR_FILENO);

            //Close every handles manually to avoid function calls
            close((*m_handles)[kPPI_StdIn].m_fds[0]);
            close((*m_handles)[kPPI_StdIn].m_fds[1]);
            close((*m_handles)[kPPI_StdOut].m_fds[0]);
            close((*m_handles)[kPPI_StdOut].m_fds[1]);
            close((*m_handles)[kPPI_StdErr].m_fds[0]);
            close((*m_handles)[kPPI_StdErr].m_fds[1]);
        } else {
            dup2(nullFd, STDIN_FILENO);
            dup2(nullFd, STDOUT_FILENO);
            dup2(nullFd, STDERR_FILENO);
            close(nullFd);
        }

        if(env == nullptr)
            execvp(args[0], const_cast<char * const *>(args));
        else
            execvpe(args[0], const_cast<char * const *>(args), const_cast<char * const *>(env));

        _exit(-2); //execvp only returns if it failed. If this happens, self-destruct.
    }

    if(m_redirSTD)
        m_handles->closeChildPipes();
    else
        close(nullFd);
    
    //Assume chdir() and exec() went fine in the child process
    if(env != nullptr)
        delete[] env;

    delete[] args;
    m_pid = static_cast<uint32_t>(pid);
    m_started = true;
#endif

    return *this;
}

m::Process &m::Process::waitFor()
{
#ifdef MGPCL_WIN
    if(m_process != INVALID_HANDLE_VALUE) {
        while(WaitForSingleObject(m_process, INFINITE) == WAIT_TIMEOUT) {
            //Well... nothing to do here I think
        }
    }
#else
    if(!m_started || m_finished)
        return *this;

    int status;
    if(waitpid(static_cast<pid_t>(m_pid), &status, 0) <= 0)
        return *this;

    m_finished = true;
    m_retCode = WEXITSTATUS(status);
#endif

    return *this;
}

int m::Process::exitCode()
{
#ifdef MGPCL_WIN
    if(m_process == INVALID_HANDLE_VALUE)
        return -1;

    DWORD ret;
    if(GetExitCodeProcess(m_process, &ret) == FALSE)
        return -1;

    return ret == STILL_ACTIVE ? -1 : static_cast<int>(ret);
#else
    if(!m_started)
        return -1;

    if(m_finished)
        return m_retCode;

    int status;
    if(waitpid(static_cast<pid_t>(m_pid), &status, WNOHANG) <= 0)
        return -1; //What shall we do if this returns 0 (if the process is still running??)
    
    m_finished = true;
    m_retCode = WEXITSTATUS(status);
    return m_retCode;
#endif
}

bool m::Process::isRunning() const
{
#ifdef MGPCL_WIN
    DWORD ret;
    if(m_process == INVALID_HANDLE_VALUE || GetExitCodeProcess(m_process, &ret) == FALSE)
        return false; //Well... what to do otherwise?

    return ret == STILL_ACTIVE;
#else
    if(!m_started || m_finished)
        return false;

    int status;
    return waitpid(static_cast<pid_t>(m_pid), &status, WNOHANG) == 0;
#endif
}

m::String m::Process::env(const String &var)
{
#ifdef MGPCL_WIN
    DWORD bufSz = GetEnvironmentVariable(var.raw(), nullptr, 0);
    if(bufSz <= 1)
        return String();

    String ret(String::uninitialized(static_cast<int>(bufSz) - 1));
    GetEnvironmentVariable(var.raw(), ret.begin(), bufSz);
    return ret;
#else
    char *str = getenv(var.raw());
    if(str == nullptr)
        return String();

    return String(str);
#endif
}

bool m::Process::enumerateProcesses(List<ProcessInfo> &lst)
{
#ifdef MGPCL_WIN
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(snap == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32 entry;
    if(Process32First(snap, &entry) == FALSE) {
        CloseHandle(snap);
        return false;
    }

    do {
        int fnameLen = 0;
        int slashPos = 0;

        while(entry.szExeFile[fnameLen] != 0) {
            if(entry.szExeFile[fnameLen] == '/' || entry.szExeFile[fnameLen] == '\\')
                slashPos = fnameLen + 1;

            fnameLen++;
        }

        lst.add(ProcessInfo(String(entry.szExeFile + slashPos, fnameLen - slashPos), entry.th32ProcessID));
    } while(Process32Next(snap, &entry) != FALSE);

    bool retVal = GetLastError() == ERROR_NO_MORE_FILES;
    CloseHandle(snap);
    return retVal;
#else
    //Gotta read /proc/
    struct dirent *entry;
    struct stat props;
    char buf[128];

    DIR *proc = opendir("/proc");
    if(proc == nullptr)
        return false;

    errno = 0;
    while((entry = readdir(proc)) != nullptr) {
        bool isPid = true;
        uint32_t parsedPid = 0;
        int fnameLen = 0;

        while(entry->d_name[fnameLen] != 0) {
            char c = entry->d_name[fnameLen++];

            if(c < '0' || c > '9') {
                isPid = false;
                break;
            } else
                parsedPid = parsedPid * 10 + static_cast<uint32_t>(c - '0');
        }

        if(fnameLen > 0 && isPid) {
            String path(fnameLen + 11); /* We have to add /proc/ (6 chars) but also /comm (5 chars) */
            path.append("/proc/", 6);
            path.append(entry->d_name, fnameLen);

            if(stat(path.raw(), &props) == 0 && S_ISDIR(props.st_mode)) {
                //This is a valid process path, try to read command name
                path.append("/comm", 5);

                int fd = open(path.raw(), O_RDONLY);
                if(fd < 0) {
                    closedir(proc);
                    return false;
                }

                String comm(8);
                while(true) {
                    ssize_t amount = read(fd, buf, 128);
                    if(amount < 0) {
                        close(fd);
                        closedir(proc);
                        return false;
                    } else if(amount == 0)
                        break; //Reached EOF

                    int commLen = 0;
                    while(static_cast<ssize_t>(commLen) < amount && buf[commLen] != '\n')
                        commLen++;

                    comm.append(buf, commLen);
                }

                //We've got everything close the 'comm' file and add it to the list
                close(fd);
                lst.add(ProcessInfo(comm, parsedPid));
            }
        }

        //Reset errno otherwise this function may return false
        errno = 0;
    }

    closedir(proc);
    return errno == 0;
#endif
}
