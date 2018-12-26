#include "mgpcl/LinuxSpecific.h"

#ifdef MGPCL_LINUX
#include "mgpcl/Process.h"
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

void m::linux::setProcessUID(Process &p, uid_t uid)
{
    p.m_setUID = true;
    p.m_targetUID = uid;
}

void m::linux::setProcessGID(Process &p, gid_t gid)
{
    p.m_setGID = true;
    p.m_targetGID = gid;
}

template<typename StructType, int SysConfCmd> StructType *getSomeID(const char *str, int(*func)(const char *, StructType *, char *, size_t, StructType **))
{
    StructType *pwInfo = new StructType;
    StructType *result = nullptr;

    long sz = sysconf(SysConfCmd);
    if(sz <= 0)
        sz = 256;

    for(int i = 0; i < 3; i++) {
        char *buf = new char[sz];
        sz <<= 1;

        int ret = func(str, pwInfo, buf, static_cast<size_t>(sz), &result);
        delete[] buf; //we only need the UID, don't care about strings

        if(ret == 0 || errno != ERANGE)
            break;
    }

    if(result == nullptr) {
        delete pwInfo;
        return nullptr;
    }

    return pwInfo;
}

bool m::linux::getUserUID(const String &uname, uid_t &dst)
{
    struct passwd *result = getSomeID<struct passwd, _SC_GETPW_R_SIZE_MAX>(uname.raw(), getpwnam_r);
    if(result == nullptr)
        return false;

    dst = result->pw_uid;
    delete result;
    return true;
}

bool m::linux::getGroupGID(const String &gname, gid_t &dst)
{
    struct group *result = getSomeID<struct group, _SC_GETGR_R_SIZE_MAX>(gname.raw(), getgrnam_r);
    if(result == nullptr)
        return false;

    dst = result->gr_gid;
    delete result;
    return true;
}

#endif
