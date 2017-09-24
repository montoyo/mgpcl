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

#include "mgpcl/File.h"

#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <climits>
#include <cstdlib>
#include <cstdio>
#include <errno.h>
#endif

m::File::File(const String &path) : m_path(path)
{
    fixPath();
}

m::File::File(const String &path, const String &fname) : m_path(path)
{
    m_path += '/'; //fixPath() has to do its job anyway...
    m_path += fname;
    fixPath();
}

m::File::File(const File &src, const String &fname) : m_path(src.m_path)
{
    m_path += '/'; //fixPath() has to do its job anyway...
    m_path += fname;
    fixPath();
}

void m::File::fixPath()
{
    if(!m_path.isEmpty()) {
        int rightTrim = m_path.length() - 1;
        while(rightTrim >= 0 && (m_path[rightTrim] == ' '  || m_path[rightTrim] == '\t' ||
                                 m_path[rightTrim] == '\r' || m_path[rightTrim] == '\n' ||
                                 m_path[rightTrim] == '/'  || m_path[rightTrim] == '\\' )) {
            rightTrim--;
        }

        rightTrim++; //Because substr() excludes 'rightTrim'

        int leftTrim = 0;
        while(leftTrim < rightTrim && (m_path[leftTrim] == ' '  || m_path[leftTrim] == '\t' ||
                                       m_path[leftTrim] == '\r' || m_path[leftTrim] == '\n' )) {
            leftTrim++;
        }

        if(leftTrim >= rightTrim) {
            m_path.cleanup();
            return;
        }

        if(rightTrim - leftTrim == 1 && m_path[leftTrim] == '.') {
            //This means 'current directory'. 
            //This is the only dot we accept.
            m_path.cleanup();
            m_path += '.';
            return;
        }

        String cpy(m_path.substr(leftTrim, rightTrim));
        m_path.cleanup();

#ifdef MGPCL_WIN
        if(cpy[0] == '/' || cpy[0] == '\\')
            return; //On windows, paths shall not begin with a slash/backslash
#endif

        int last = 0;
        int i = 0;

        while(i < cpy.length()) {
            if(cpy[i] == '/' || cpy[i] == '\\') {
                if(last < i) {
                    if(last == i - 1 && cpy[last] == '.') {
                        //Turn things like 'foo/./bar' into 'foo/bar'
                        last = ++i;
                        continue;
                    }
                    
                    m_path += cpy.substr(last, i);
                }

#ifdef MGPCL_WIN
                m_path += '\\';
#else
                m_path += '/';
#endif

                while(i < cpy.length() && (cpy[i] == '/' || cpy[i] == '\\')) //Technically, because of the previous trim, 'i < cpy.length()' is ALWAYS true.
                    i++;

                last = i++; //Ignore the next character as it won't be a slash/backslash
            } else
                i++;
        }

        if(last == cpy.length() - 1 && cpy[last] == '.') {
            //It's a remaining dot; don't add anything and remove the trailing slash/backslash
#ifdef MGPCL_LINUX
            if(m_path.length() > 1 || m_path[0] != '/')
                m_path.trimToLength(m_path.length() - 1);
#else
            m_path.trimToLength(m_path.length() - 1);
#endif
        } else
            m_path += cpy.substr(last);
    }
}

bool m::File::exists() const
{
#ifdef MGPCL_WIN
    return GetFileAttributes(m_path.raw()) != INVALID_FILE_ATTRIBUTES || GetLastError() != ERROR_FILE_NOT_FOUND;
#else
    return access(m_path.raw(), F_OK) == 0;
#endif
}

bool m::File::isDirectory() const
{
#ifdef MGPCL_WIN
    return (GetFileAttributes(m_path.raw()) & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat s;
    if(stat(m_path.raw(), &s) != 0)
            return false;

    return S_ISDIR(s.st_mode);
#endif
}

bool m::File::isFile() const
{
#ifdef MGPCL_WIN
    return (GetFileAttributes(m_path.raw()) & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
    struct stat s;
    if(stat(m_path.raw(), &s) != 0)
            return false;

    return !S_ISDIR(s.st_mode);
#endif
}

bool m::File::mkdir() const
{
#ifdef MGPCL_WIN
    return CreateDirectory(m_path.raw(), nullptr) != FALSE || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return ::mkdir(m_path.raw(), 0755) == 0;
#endif
}

bool m::File::mkdirs() const
{
#ifdef MGPCL_WIN
    int ret = SHCreateDirectoryEx(nullptr, m_path.raw(), nullptr);
    return ret == ERROR_SUCCESS || ret == ERROR_FILE_EXISTS || ret == ERROR_ALREADY_EXISTS;
#else
    struct stat s;
    String tmp;

    for(int i = 1; i < m_path.length(); i++) {
        if(m_path[i] == '/') {
            tmp = m_path.substr(0, i);
            
            //Even if m_path contains ".." that shouldn't be a problem;
            //we just may repeat one operation twice

            if(stat(tmp.raw(), &s) != 0 || !S_ISDIR(s.st_mode)) {
                if(::mkdir(tmp.raw(), 0755) != 0)
                    return false;
            }
        }
    }

    return true;
#endif
}

m::File &m::File::canonicalize()
{
#ifdef MGPCL_WIN
    //Windows fails to canonicalize if this is a relative path
    if(m_path.length() == 1 && m_path[0] == '.') {
        m_path = std::move(workingDirectory().m_path);
        return *this;
    }
    
    if(m_path.length() < 2 || m_path[1] != ':')
        m_path = workingDirectory().m_path + '\\' + m_path;

    char tmp[MAX_PATH];
    if(PathCanonicalize(tmp, m_path.raw()) == FALSE)
        m_path.clear();
    else
        m_path = tmp;
#else
    char tmp[PATH_MAX];
    if(realpath(m_path.raw(), tmp) == nullptr)
        m_path.clear();
    else
        m_path = tmp;
#endif

    return *this;
}

bool m::File::listFiles(std::function<void(const File &)> cb) const
{
    FileIterator fi(begin());
    for(; fi != end(); ++fi)
        cb(*fi);

    return !fi.errored();
}

bool m::File::listFiles(List<File> &dst) const
{
    FileIterator fi(begin());
    for(; fi != end(); ++fi)
        dst.add(*fi);

    return !fi.errored();
}

bool m::File::listFilesRecursive(List<File> &dst) const
{
    FileIterator fi(begin());
    for(; fi != end(); ++fi) {
        dst.add(*fi);

        if(fi->isDirectory() && !fi->listFilesRecursive(dst))
            return false;
    }

    return !fi.errored();
}

m::File m::File::workingDirectory()
{
#ifdef MGPCL_WIN
    char tmp[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, tmp);
#else
    char tmp[PATH_MAX];
    if(getcwd(tmp, PATH_MAX) == nullptr)
        return File();
#endif

    File ret;
    ret.m_path = tmp;
    return ret;
}

m::File m::File::usualDirectory(UsualDirectories ud)
{
#ifdef MGPCL_WIN
    KNOWNFOLDERID kfi;
    switch(ud) {
    case kUD_UserDesktop:
        kfi = FOLDERID_Desktop;
        break;

    case kUD_UserDocuments:
        kfi = FOLDERID_Documents;
        break;

    case kUD_UserPictures:
        kfi = FOLDERID_Pictures;
        break;

    case kUD_UserMusic:
        kfi = FOLDERID_Music;
        break;

    case kUD_UserVideos:
        kfi = FOLDERID_Videos;
        break;

    case kUD_UserDownloads:
        kfi = FOLDERID_Downloads;
        break;

    case kUD_UserHome:
        kfi = FOLDERID_Profile;
        break;

    case kUD_UserAppData:
        kfi = FOLDERID_RoamingAppData;
        break;

    case kUD_UserTemplates:
        kfi = FOLDERID_Templates;
        break;

    case kUD_SystemFonts:
        kfi = FOLDERID_Fonts;
        break;

    default:
        return File();
    }

    wchar_t *wstr;
    if(SHGetKnownFolderPath(kfi, KF_FLAG_DEFAULT, nullptr, &wstr) != S_OK)
        return File();

    size_t len = wcslen(wstr);
    size_t converted;

    char *str = new char[len * sizeof(wchar_t) + 1];
    if(wcstombs_s(&converted, str, len * sizeof(wchar_t) + 1, wstr, _TRUNCATE) != 0) {
        CoTaskMemFree(wstr);
        delete[] str;

        return File();
    }

    File ret;
    ret.m_path.append(str, static_cast<int>(converted));

    CoTaskMemFree(wstr);
    delete[] str;

    return ret;
#else
    //TODO: Make this AT LEAST politically correct...

    if(ud == kUD_SystemFonts) {
        File ret;
        ret.m_path.append("/usr/share/fonts", 16);

        return ret;
    }

    long bufsz = sysconf(_SC_GETPW_R_SIZE_MAX);
    if(bufsz <= 0)
        bufsz = 256;

    int uret;
    char *buf = new char[bufsz];
    struct passwd pswd;
    struct passwd *result = nullptr;

    while(bufsz < 10 * 1024) {
        uret = getpwuid_r(getuid(), &pswd, buf, static_cast<size_t>(bufsz), &result);
        if(uret != ERANGE)
            break;

        //Buffer wasn't large enough; reallocate...
        bufsz += 256;
        delete[] buf;
        buf = new char[bufsz];
    }

    if(uret != 0 || result == nullptr) {
        delete[] buf;
        return File();
    }

    String userHome(result->pw_dir);
    delete[] buf;

    File ret;
    ret.m_path += userHome;

    if(ud == kUD_UserHome)
        return ret;

    switch(ud) {
    case kUD_UserDesktop:
        ret.m_path.append("/Desktop", 8);
        break;

    case kUD_UserDocuments:
        ret.m_path.append("/Documents", 10);
        break;

    case kUD_UserPictures:
        ret.m_path.append("/Pictures", 9);
        break;

    case kUD_UserMusic:
        ret.m_path.append("/Music", 6);
        break;

    case kUD_UserVideos:
        ret.m_path.append("/Videos", 7);
        break;

    case kUD_UserDownloads:
        ret.m_path.append("/Downloads", 10);
        break;

    case kUD_UserAppData:
        ret.m_path.append("/.appdata", 9);
        break;

    case kUD_UserTemplates:
        ret.m_path.append("/Templates", 10);
        break;

    default:
        return File();
    }

    return ret;
#endif
}

m::File m::File::parent() const
{
    if(m_path.isEmpty())
        return *this;

#ifdef MGPCL_WIN
    if(m_path.length() == 2 && m_path[1] == ':')
        return File(); //Drive root has no parent

    if(m_path.endsWith("\\..", 3)) {
        File f(*this);
        f.m_path.append("\\..", 3);
        return f;
    }

    int slash = m_path.lastIndexOf('\\');
#else
    if(m_path.endsWith("/..", 3)) {
        File f(*this);
        f.m_path.append("/..", 3);
        return f;
    }

    int slash = m_path.lastIndexOf('/');
    if(slash == 0) {
        if(m_path.length() > 1) {
            //FIXME: Assuming no one tries something like '/..'; gotta check for this in fixPath()
            File f;
            f.m_path += '/';

            return f;
        } else
            return File();
    }
#endif

    if(slash < 0) {
        //No slashes in this path
        if(m_path.length() == 1 && m_path[0] == '.') {
            File f;
            f.m_path.append("..", 2); //avoid call to fixPath()

            return f;
        } else {
            File f(*this);

#ifdef MGPCL_WIN
            f.m_path.append("\\..", 3); //avoid call to fixPath()
#else
            f.m_path.append("/..", 3); //avoid call to fixPath()
#endif

            return f;
        }
    }

    File f;
    f.m_path = m_path.substr(0, slash);
    return f;
}

m::String m::File::fileName() const
{
    if(isEmpty())
        return String();

#ifdef MGPCL_WIN
    int pos = m_path.lastIndexOf('\\');
#else
    int pos = m_path.lastIndexOf('/');
#endif

    if(pos < 0)
        return m_path;
    else
        return m_path.substr(pos + 1);
}

m::String m::File::extension() const
{
    if(isEmpty())
        return String();

#ifdef MGPCL_WIN
    int slashPos = m_path.lastIndexOf('\\');
#else
    int slashPos = m_path.lastIndexOf('/');
#endif

    int pos = m_path.lastIndexOf('.');

    if(pos < slashPos)
        return String();
    else
        return m_path.substr(pos + 1);
}

m::String m::File::fileNameExtensionless() const
{
    if(isEmpty())
        return String();

#ifdef MGPCL_WIN
    int slashPos = m_path.lastIndexOf('\\') + 1;
#else
    int slashPos = m_path.lastIndexOf('/') + 1;
#endif

    int pos = m_path.lastIndexOf('.');

    if(pos < slashPos)
        return m_path.substr(slashPos);
    else
        return m_path.substr(slashPos, pos);
}

m::String m::File::pathExtensionless() const
{
    if(isEmpty())
        return String();

#ifdef MGPCL_WIN
    int slashPos = m_path.lastIndexOf('\\') + 1;
#else
    int slashPos = m_path.lastIndexOf('/') + 1;
#endif

    int pos = m_path.lastIndexOf('.');

    if(pos < slashPos)
        return m_path;
    else
        return m_path.substr(0, pos);
}

bool m::File::renameTo(const String &nf)
{
    if(isEmpty() || nf.indexOfAnyOf("/\\", 0, 2) >= 0)
        return false;

#ifdef MGPCL_WIN
    int pos = m_path.lastIndexOf('\\');
#else
    int pos = m_path.lastIndexOf('/');
#endif

    if(pos < 0) {
        File c(*this);
        c.canonicalize();
        if(c.isEmpty()) //Failed
            return false;

        m_path = c.m_path;

#ifdef MGPCL_WIN
        pos = m_path.lastIndexOf('\\');
#else
        pos = m_path.lastIndexOf('/');
#endif

        if(pos < 0)
            return false; //Well this should have worked...
    }

    String npath(m_path.substr(0, pos));
    npath += nf;

#ifdef MGPCL_WIN
    if(MoveFile(m_path.raw(), npath.raw()) == FALSE)
        return false;
#else
    if(rename(m_path.raw(), npath.raw()) != 0)
        return false;
#endif

    m_path = npath;
    return true;
}

bool m::File::moveTo(const File &nf) const
{
#ifdef MGPCL_WIN
    return MoveFile(m_path.raw(), nf.m_path.raw()) != FALSE;
#else
    return rename(m_path.raw(), nf.m_path.raw()) == 0;
#endif
}

bool m::File::deleteFile() const
{
#ifdef MGPCL_WIN
    return DeleteFile(m_path.raw()) != FALSE;
#else
    return unlink(m_path.raw()) == 0;
#endif
}

bool m::File::copyTo(const File &nf) const
{
#ifdef MGPCL_WIN
    return CopyFile(m_path.raw(), nf.m_path.raw(), FALSE) == 0;
#else
    int src = open(m_path.raw(), O_RDONLY);
    if(src < 0)
        return false;

    int dst = open(nf.m_path.raw(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(dst < 0)
        return false;

    uint8_t *buf = new uint8_t[65536];
    ssize_t rd = read(src, buf, 65536);

    while(rd > 0) {
        write(dst, buf, static_cast<size_t>(rd));
        rd = read(src, buf, 65536);
    }

    delete[] buf;
    close(dst);
    close(src);

    return rd == 0;
#endif
}

bool m::File::deleteFileHarder() const
{
    if(deleteFile())
        return true;

    //Couldn't delete; find available name
    String deleteMe("deleteMe", 8);
    File p(parent());
    File f(p, deleteMe);

    for(uint32_t i = 0; f.exists(); i++)
        f = File(p, deleteMe + String::fromUInteger(i));

    if(!moveTo(f))
        return false; //Couldn't even move! This is the end...

    f.deleteFile(); //Try to delete, but we don't care if it worked or not...
    return true;
}

m::FileIterator::FileIterator()
{
    m_err = false;

#ifdef MGPCL_WIN
    m_data = INVALID_HANDLE_VALUE;
#else
    m_data = nullptr;
#endif
}

m::FileIterator::~FileIterator()
{
#ifdef MGPCL_WIN
    if(m_data != INVALID_HANDLE_VALUE)
        FindClose(m_data);
#else
    if(m_data != nullptr)
        closedir(static_cast<DIR*>(m_data));
#endif
}

m::FileIterator::FileIterator(FileIterator &&src)
{
    m_err = src.m_err;
    m_data = src.m_data;
    m_root = src.m_root;
    m_last = src.m_last;

#ifdef MGPCL_WIN
    src.m_data = INVALID_HANDLE_VALUE;
#else
    src.m_data = nullptr;
#endif
}

m::FileIterator::FileIterator(const String &path)
{
#ifdef MGPCL_WIN
    int len = path.length();
    while(len > 0 && (path[len - 1] == '/' || path[len - 1] == '\\'))
        len--;

    if(len <= 0 || len + 3 >= MAX_PATH) {
        m_data = INVALID_HANDLE_VALUE;
        m_err = true;
        return;
    }

    char tmp[MAX_PATH];
    Mem::copy(tmp, path.raw(), len);
    tmp[len + 0] = '\\';
    tmp[len + 1] = '*';
    tmp[len + 2] = 0;

    WIN32_FIND_DATA data;
    HANDLE find = FindFirstFile(tmp, &data);
    bool keepGoing = true;

    if(find == INVALID_HANDLE_VALUE) {
        m_data = INVALID_HANDLE_VALUE;
        m_err = true;
        return;
    }

    do {
        if(isValid(data.cFileName)) {
            m_last = File(path, String(data.cFileName));
            keepGoing = false;
        }
    } while(keepGoing && FindNextFile(find, &data) != FALSE);

    if(keepGoing) {
        //Nothing has been found
        m_err = GetLastError() != ERROR_NO_MORE_FILES;
        m_data = INVALID_HANDLE_VALUE;
        FindClose(find);
        return;
    }

    m_err = false;
    m_root = path;
    m_data = find;
#else
    DIR *d = opendir(path.raw());
    struct dirent *dir;

    if(d == nullptr) {
        m_data = nullptr;
        m_err = true;
        return;
    }

    bool keepGoing = true;
    errno = 0;

    while(keepGoing && (dir = readdir(d)) != nullptr) {
        if(isValid(dir->d_name)) {
            m_last = File(path, String(dir->d_name));
            keepGoing = false;
        }
    }

    if(keepGoing) {
        //Nothing has been found
        m_err = errno != 0;
        m_data = nullptr;
        closedir(d);
        return;
    }

    m_err = false;
    m_root = path;
    m_data = d;
#endif
}

m::FileIterator &m::FileIterator::operator++ ()
{
#ifdef MGPCL_WIN
    if(m_data == INVALID_HANDLE_VALUE)
        return *this;

    WIN32_FIND_DATA data;
    bool keepGoing = true;

    while(keepGoing && FindNextFile(m_data, &data) != FALSE) {
        if(isValid(data.cFileName)) {
            m_last = File(m_root, String(data.cFileName));
            keepGoing = false;
        }
    }

    if(keepGoing) {
        //Find ended
        m_err = GetLastError() != ERROR_NO_MORE_FILES;
        FindClose(m_data);
        m_data = INVALID_HANDLE_VALUE;
    }
#else
    if(m_data == nullptr)
        return *this;

    struct dirent *dir;
    bool keepGoing = true;
    errno = 0;

    while(keepGoing && (dir = readdir(static_cast<DIR*>(m_data))) != nullptr) {
        if(isValid(dir->d_name)) {
            m_last = File(m_root, String(dir->d_name));
            keepGoing = false;
        }
    }

    if(keepGoing) {
        //Find ended
        m_err = errno != 0;
        closedir(static_cast<DIR*>(m_data));
        m_data = nullptr;
    }
#endif

    return *this;
}

m::FileIterator &m::FileIterator::operator = (FileIterator &&src)
{
#ifdef MGPCL_WIN
    if(m_data != INVALID_HANDLE_VALUE)
        FindClose(m_data);
#else
    if(m_data != nullptr)
        closedir(static_cast<DIR*>(m_data));
#endif

    m_err = src.m_err;
    m_data = src.m_data;
    m_root = src.m_root;
    m_last = src.m_last;

#ifdef MGPCL_WIN
    src.m_data = INVALID_HANDLE_VALUE;
#else
    src.m_data = nullptr;
#endif

    return *this;
}

bool m::FileIterator::isValid(const char *file)
{
    return strcmp(file, ".") != 0 && strcmp(file, "..") != 0;
}
