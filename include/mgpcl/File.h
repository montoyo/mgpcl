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
#include <cstdlib>
#include <functional>

namespace m
{
    class FileIterator;

    enum UsualDirectories
    {
        kUD_UserDesktop,
        kUD_UserDocuments,
        kUD_UserPictures,
        kUD_UserMusic,
        kUD_UserVideos,
        kUD_UserDownloads,
        kUD_UserHome,
        kUD_UserAppData,
        kUD_UserTemplates,
        kUD_SystemFonts
    };

    class MGPCL_PREFIX File
    {
    public:
        File()
        {
        }

        File(const String &path);
        File(const String &path, const String &fname);
        File(const File &src, const String &fname);

        File(const File &src) : m_path(src.m_path)
        {
        }

        File(File &&src) : m_path(std::move(src.m_path))
        {
        }

        ~File()
        {
        }

        File &operator = (const File &src)
        {
            m_path = src.m_path;
            return *this;
        }

        File &operator = (File &&src)
        {
            m_path = std::move(src.m_path);
            return *this;
        }

        bool exists() const;
        bool isDirectory() const;
        bool isFile() const; //Same as !isDirectory()

        File parent() const;
        String fileName() const;
        String extension() const;
        String fileNameExtensionless() const;
        String pathExtensionless() const;

        bool mkdir() const;
        bool mkdirs() const;

        inline FileIterator begin() const;
        inline FileIterator end() const;

        bool listFiles(std::function<void(const File &)> cb) const;
        bool listFiles(List<File> &dst) const;
        bool listFilesRecursive(List<File> &dst) const;

        //This will only change the filename. Thus, 'fname' should not contain any slashes.
        //Note that this will change the path of this 'File' instance AND will ALSO rename
        //the filename of the file represented by this path on the filesystem.
        bool renameTo(const String &fname);
        bool copyTo(const File &nf) const;
        bool moveTo(const File &nf) const;
        bool deleteFile() const;

        //This function will try to delete the file. If it can't,
        //it'll try to rename it and then delete it.
        //
        //It'll return true if the path represented by this File
        //instance doesn't point to a file anymore (if the deletion
        //succeeded or if the file was moved)
        bool deleteFileHarder() const;

        File &canonicalize();
        static File workingDirectory();
        static File usualDirectory(UsualDirectories ud); //For now, usual directory paths for Linux are hard-coded...

        bool isEmpty() const
        {
            return m_path.isEmpty();
        }

        const String &path() const
        {
            return m_path;
        }

        void setPath(const String &src)
        {
            m_path = src;
            fixPath();
        }

        File &operator = (const String &src)
        {
            m_path = src;
            fixPath();
            return *this;
        }

        bool isAbsolute() const
        {
#ifdef MGPCL_WIN
            if(m_path.length() == 2)
                return m_path[1] == ':';
            else if(m_path.length() > 2)
                return m_path[1] == ':' && m_path[2] == '\\';
            else
                return false;
#else
            return m_path.length() >= 1 && m_path[0] == '/';
#endif
        }

        bool isRelative() const
        {
#ifdef MGPCL_WIN
            if(m_path.length() == 2)
                return m_path[1] != ':';
            else if(m_path.length() > 2)
                return m_path[1] != ':' || m_path[2] != '\\';
            else
                return true;
#else
            return m_path.length() < 1 || m_path[0] != '/';
#endif
        }

        File canonicalized() const
        {
            return File(*this).canonicalize();
        }

        static char separator()
        {
#ifdef MGPCL_WIN
            return '\\';
#else
            return '/';
#endif
        }

    private:
        void fixPath();
        String m_path;
    };

    class FileIterator
    {
        friend class File;

    public:
        FileIterator(FileIterator &&src);
        ~FileIterator();

        File *operator -> ()
        {
            return &m_last;
        }

        const File *operator -> () const
        {
            return &m_last;
        }

        File &operator * ()
        {
            return m_last;
        }

        const File &operator * () const
        {
            return m_last;
        }

        bool operator == (const FileIterator &src) const
        {
            return m_data == src.m_data;
        }

        bool operator != (const FileIterator &src) const
        {
            return m_data != src.m_data;
        }

        bool errored() const
        {
            return m_err;
        }

        FileIterator &operator++ ();
        FileIterator &operator = (FileIterator &&src);

    private:
        FileIterator(const FileIterator &src)
        {
            //CANNOT be copied!
            std::abort();
        }

        FileIterator &operator = (const FileIterator &src)
        {
            //CANNOT be copied!
            std::abort();
        }

        FileIterator();
        FileIterator(const String &path);

        static bool isValid(const char *file);

        bool m_err;
        void *m_data;
        String m_root;
        File m_last;
    };

    inline FileIterator File::begin() const
    {
        return FileIterator(m_path);
    }

    inline FileIterator File::end() const
    {
        return FileIterator();
    }

}
