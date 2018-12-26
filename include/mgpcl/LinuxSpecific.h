#pragma once
#include "String.h"

#ifdef MGPCL_LINUX
#include <unistd.h>

#ifdef linux
#undef linux
#endif

#ifdef M_LSPEC_C
#define M_LSPEC_C
#else
#define M_LSPEC_C extern
#endif

namespace m
{
    class Process;

    namespace linux
    {
        M_LSPEC_C void setProcessUID(Process &p, uid_t uid);
        M_LSPEC_C void setProcessGID(Process &p, gid_t gid);
        M_LSPEC_C bool getUserUID(const String &uname, uid_t &dst);
        M_LSPEC_C bool getGroupGID(const String &gname, gid_t &dst);

        inline uid_t getCurrentUID()
        {
            return getuid();
        }

        inline gid_t getCurrentGID()
        {
            return getgid();
        }
    }
}

#endif
