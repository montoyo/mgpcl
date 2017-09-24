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

#include "mgpcl/WinCmdLine.h"
#include "mgpcl/ReadWriteLock.h"

/*
 * WARNING
 * This is a ugly hack, enabling me to get processes command lines on Windows.
 * Somehow there is no official API for this, so until Microsoft writes one,
 * well... I'm going to use this.
 * 
 * Hopefully no one will get VAC banned from this, because it reads into processes
 * memory.
 * 
 * Also the structures I took here from MSDN were made for x86_64, so not sure
 * if this is going to work on x86...
 * 
 * ~montoyo
 */

#ifdef MGPCL_WIN

/************************************************ WIN INTERNAL TYPES *************************************************************/
typedef struct _PEB_LDR_DATA
{
    BYTE       Reserved1[8];
    PVOID      Reserved2[3];
    LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _RTL_USER_PROCESS_PARAMETERS
{
    BYTE           Reserved1[16];
    PVOID          Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef void *PPS_POST_PROCESS_INIT_ROUTINE; //I assume...
typedef struct _PEB //This structure is for Windows x64, according to MSDN docs...
{
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[21];
    PPEB_LDR_DATA LoaderData;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    BYTE Reserved3[520];
    PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
    BYTE Reserved4[136];
    ULONG SessionId;
} PEB, *PPEB;

typedef struct _PROCESS_BASIC_INFORMATION
{
    PVOID Reserved1;
    PPEB PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

typedef enum _PROCESSINFOCLASS  //I assume...
{
    ProcessBasicInformation = 0,
    ProcessDebugPort = 7,
    ProcessWow64Information = 26,
    ProcessImageFileName = 27,
    ProcessBreakOnTermination = 29
} PROCESSINFOCLASS, *PPROCESSINFOCLASS;

typedef LONG (WINAPI *ProcNtQueryInformationProcess)(HANDLE proc, PROCESSINFOCLASS cls, PVOID pInfo, ULONG infoLen, PULONG retLen);

/************************************************ UTILITY GLOBALS *************************************************************/
static volatile bool g_loaded = false;
static ProcNtQueryInformationProcess NtQueryInformationProcess = nullptr;
static m::ReadWriteLock g_lock;

/************************************************ UTILITY FUNCTIONS *************************************************************/
static bool _remoteRead(HANDLE proc, void *addr, void *dst, SIZE_T sz)
{
    SIZE_T rd = sz;
    return ReadProcessMemory(proc, addr, dst, sz, &rd) != FALSE && rd == sz;
}

template<typename T> bool remoteRead(HANDLE proc, T *addr, T *dst)
{
    return _remoteRead(proc, addr, dst, sizeof(T));
}

static bool _initIfNeeded()
{
    //Give debug rights to current process.
    HANDLE tok;
    TOKEN_PRIVILEGES tp;
    if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tok) == FALSE) {
        OutputDebugString("Could not adjust current process privileges");
        return false;
    }

    if(LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &tp.Privileges[0].Luid) == FALSE) {
        OutputDebugString("Could not lookup privilege value");
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if(AdjustTokenPrivileges(tok, FALSE, &tp, 0, nullptr, nullptr) == FALSE) {
        OutputDebugString("Could not enable DEBUG privilege");
        return false;
    }

    //Load proc address
    HMODULE ntdll = LoadLibrary("ntdll.dll");
    if(ntdll == nullptr) {
        OutputDebugString("Could not load ntdll.dll");
        return false;
    }

    NtQueryInformationProcess = reinterpret_cast<ProcNtQueryInformationProcess>(GetProcAddress(ntdll, "NtQueryInformationProcess"));
    if(NtQueryInformationProcess == nullptr) {
        OutputDebugString("Could not get proc address of NtQueryInformationProcess");
        return false;
    }

    return true;
}

bool m::win::initIfNeeded()
{
    g_lock.lockFor(RWAction::Reading);
    volatile bool loaded = g_loaded;
    volatile bool result = NtQueryInformationProcess != nullptr;
    g_lock.releaseFor(RWAction::Reading);

    if(loaded)
        return result;

    g_lock.lockFor(RWAction::Writing);
    if(g_loaded) {
        //It may be loaded in the meantime (while g_lock was unlocked), so double-check here
        result = NtQueryInformationProcess != nullptr;
        g_lock.releaseFor(RWAction::Writing);

        return result;
    }

    bool ret = _initIfNeeded();
    g_loaded = true;
    g_lock.releaseFor(RWAction::Writing);
    return ret;
}

/************************************************ ACTUAL FUNCTION *************************************************************/
bool m::win::getProcessCommandLine(HANDLE proc, String &cmd)
{
    PROCESS_BASIC_INFORMATION infos;
    ULONG pbeLen = sizeof(PROCESS_BASIC_INFORMATION);

    NtQueryInformationProcess(proc, ProcessBasicInformation, &infos, sizeof(PROCESS_BASIC_INFORMATION), &pbeLen);
    if(pbeLen != sizeof(PROCESS_BASIC_INFORMATION)) {
        OutputDebugString("NtQueryInformationProcess() failed!");
        return false;
    }

    if(infos.PebBaseAddress == nullptr) {
        OutputDebugString("PebBaseAddress is null!");
        return false;
    }

    PEB peb;
    if(!remoteRead(proc, infos.PebBaseAddress, &peb)) {
        OutputDebugString("Could not read PEB");
        return false;
    }

    if(peb.ProcessParameters == nullptr) {
        OutputDebugString("ProcessParameters is null!");
        return false;
    }

    RTL_USER_PROCESS_PARAMETERS params;
    if(!remoteRead(proc, peb.ProcessParameters, &params)) {
        OutputDebugString("Could not read RTL_USER_PROCESS_PARAMETERS");
        return false;
    }

    if(params.CommandLine.Length == 0)
        return true; //String is empty, so no need to fill clData

    SIZE_T strRd;
    USHORT wcharCnt = params.CommandLine.Length / sizeof(wchar_t);
    wchar_t *clData = new wchar_t[wcharCnt + 1];

    if(ReadProcessMemory(proc, params.CommandLine.Buffer, clData, params.CommandLine.Length, &strRd) == FALSE || strRd != static_cast<SIZE_T>(params.CommandLine.Length)) {
        OutputDebugString("Could not read CommandLine.Buffer");
        delete[] clData;
        return false;
    }

    clData[wcharCnt] = 0;

    size_t retLen;
    char *dst = new char[wcharCnt + 1];
    wcstombs_s(&retLen, dst, wcharCnt + 1, clData, _TRUNCATE);
    delete[] clData;

    //cmd.cleanup();
    cmd.append(dst, static_cast<int>(retLen) - 1);
    delete[] dst;
    return true;
}

#endif
