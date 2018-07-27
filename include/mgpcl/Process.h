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
#include "String.h"
#include "List.h"
#include "HashMap.h"
#include "IOStream.h"
#include "Atomic.h"
#include "SharedPtr.h"
#include <initializer_list>

#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace m
{
    class Process;

    class ProcessInfo
    {
        friend class Process;

    public:
        const String &commandName() const
        {
            return m_cmdName;
        }

        uint32_t pid() const
        {
            return m_pid;
        }

        bool commandLine(String &dst) const;

    private:
        ProcessInfo()
        {
        }

        ProcessInfo(const String &n, uint32_t pid) : m_cmdName(n)
        {
            m_pid = pid;
        }

        ProcessInfo(String &&n, uint32_t pid) : m_cmdName(n)
        {
            m_pid = pid;
        }

        String m_cmdName;
        uint32_t m_pid;
    };

    class PipeInputStream;
    class PipeOutputStream;

    class ProcessPipe
    {
        friend class Process;
        friend class PipeInputStream;
        friend class PipeOutputStream;

    public:
        enum PipeDirection
        {
            kPPD_None = 0,
            kPPD_ChildInput, //STDIN
            kPPD_ChildOutput //STDOUT, STDERR
        };

        ProcessPipe();
        bool create(PipeDirection dir);
        void closeChildPipe();
        void destroy();

    private:
        PipeDirection m_dir;

#ifdef MGPCL_WIN
        HANDLE m_rd;
        HANDLE m_wr;
#else
        int m_fds[2]; //0 = read, 1 = write
#endif
    };

    enum ProcessPipeID
    {
        kPPI_StdIn = 0,
        kPPI_StdOut,
        kPPI_StdErr,
        kPPI_Count
    };

    class ProcessPipes
    {
    public:
        ProcessPipes();

        bool createPipes();
        void closeChildPipes();
        void addRef();
        void releaseRef();

        ProcessPipe &operator[] (ProcessPipeID ppid)
        {
            return m_pipes[ppid];
        }

    private:
        ~ProcessPipes()
        {
        }

        Atomic m_refs;
        ProcessPipe m_pipes[kPPI_Count];
    };

    class PipeInputStream : public InputStream
    {
        M_NON_COPYABLE(PipeInputStream)

    public:
        PipeInputStream(ProcessPipes *handles, ProcessPipeID pid);
        PipeInputStream(PipeInputStream &&src);
        ~PipeInputStream() override;

        int read(uint8_t *dst, int sz) override;
        uint64_t pos() override;
        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override;
        bool seekSupported() const override;
        void close() override;

        PipeInputStream &operator = (PipeInputStream &&src);

    private:
        PipeInputStream()
        {
        }

        ProcessPipes *m_handles;
        ProcessPipeID m_pipe;
        uint64_t m_pos;
    };

    class PipeOutputStream : public OutputStream
    {
        M_NON_COPYABLE(PipeOutputStream)

    public:
        PipeOutputStream(ProcessPipes *handles, ProcessPipeID pid);
        PipeOutputStream(PipeOutputStream &&src);
        ~PipeOutputStream() override;

        int write(const uint8_t *src, int sz) override;
        uint64_t pos() override;
        bool seek(int amount, SeekPos sp = SeekPos::Beginning) override;
        bool seekSupported() const override;
        bool flush() override;
        void close() override;

        PipeOutputStream &operator = (PipeOutputStream &&src);

    private:
        PipeOutputStream()
        {
        }

        ProcessPipes *m_handles;
        ProcessPipeID m_pipe;
        uint64_t m_pos;
    };

    class Process
    {
        M_NON_COPYABLE(Process)

    public:
#ifdef MGPCL_WIN
        typedef HashMap<String, String, StringLowerHasher> EnvMap;
#else
        typedef HashMap<String, String> EnvMap;
#endif

        Process();
        ~Process();
        Process &start();
        Process &waitFor();
        bool isRunning() const;
        int exitCode();

        Process &setWorkingDirectory(const String &wd)
        {
            m_workingDir = wd;
            return *this;
        }

        Process &setExecutable(const String &exe)
        {
            m_exe = exe;
            return *this;
        }

        Process &pushArg(const String &a)
        {
            m_args.add(a);
            return *this;
        }

        Process &clearArgs()
        {
            m_args.clear();
            return *this;
        }

        Process &pushArgs(std::initializer_list<String> lst)
        {
            for(const String &s : lst)
                m_args.add(s);

            return *this;
        }

        Process &setEnv(const String &key, const String &val)
        {
            m_env[key] = val;
            return *this;
        }

        Process &redirectSTDIO(bool redir = true)
        {
            m_redirSTD = redir;
            return *this;
        }

        bool doesRedirectSTDIO() const
        {
            return m_redirSTD;
        }

        String cenv(const String &key) const
        {
            //Child Env != (Parent) Env
            return m_env[key];
        }

        const String &workingDirectory() const
        {
            return m_workingDir;
        }

        const String &executable() const
        {
            return m_workingDir;
        }

        const String &arg(int id) const
        {
            return m_args[id];
        }

        int argCount() const
        {
            return m_args.size();
        }

        uint32_t pid() const
        {
            return m_pid;
        }

        bool hasStarted() const
        {
#ifdef MGPCL_WIN
            return m_process != INVALID_HANDLE_VALUE;
#else
            return m_started;
#endif
        }

        template<class RefCnt> SharedPtr<PipeOutputStream, RefCnt> stdIn()
        {
            if(!m_redirSTD || !hasStarted())
                return SharedPtr<PipeOutputStream, RefCnt>();

            return SharedPtr<PipeOutputStream, RefCnt>(new PipeOutputStream(m_handles, kPPI_StdIn));
        }

        template<class RefCnt> SharedPtr<PipeInputStream, RefCnt> stdOut()
        {
            if(!m_redirSTD || !hasStarted())
                return SharedPtr<PipeInputStream, RefCnt>();

            return SharedPtr<PipeInputStream, RefCnt>(new PipeInputStream(m_handles, kPPI_StdOut));
        }

        template<class RefCnt> SharedPtr<PipeInputStream, RefCnt> stdErr()
        {
            if(!m_redirSTD || !hasStarted())
                return SharedPtr<PipeInputStream, RefCnt>();

            return SharedPtr<PipeInputStream, RefCnt>(new PipeInputStream(m_handles, kPPI_StdErr));
        }

        static String env(const String &var); //Parent env = this process' env
        static bool enumerateProcesses(List<ProcessInfo> &lst);
        static uint32_t getCurrentProcessID();

    private:
        String m_workingDir;
        String m_exe;
        List<String> m_args;
        EnvMap m_env;
        uint32_t m_pid;
        bool m_redirSTD;

        ProcessPipes *m_handles;

#ifdef MGPCL_WIN
        HANDLE m_process;
#else
        bool m_started;
        bool m_finished;
        int m_retCode;
#endif
    };
}
