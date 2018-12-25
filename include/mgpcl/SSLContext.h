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
#include "RefCounter.h"
#include "String.h"
#include "Config.h"

#ifndef MGPCL_NO_SSL

namespace m
{
    enum SSLContextMethod
    {
        kSCM_v23Method,
        kSCM_v23Client,
        kSCM_v23Server
    };

    enum SSLVerifyFlags
    {
        kSVF_None = 0,
        kSVF_VerifyPeer = 1,
        kSVF_FailIfNoPeerCert = 2,
        kSVF_VerifyClientOnce = 4
    };

    class SSLContext
    {
    public:
        SSLContext();
        SSLContext(const SSLContext &src);
        SSLContext(SSLContext &&src);
        SSLContext(SSLContextMethod method);
        ~SSLContext();

        bool initialize(SSLContextMethod method);
        bool loadVerifyPath(const String &path);
        bool loadVerifyFile(const String &file);
        bool loadOSVerify();
        bool setVerifyFlags(int flags);
        bool setVerifyDepth(int depth);
        bool enableAutoECDH(bool autoECDH);
        bool useCertificateFile(const String &file);
        bool usePrivateKeyFile(const String &file);

        SSLContext &operator = (const SSLContext &src);
        SSLContext &operator = (SSLContext &&src);

        void *raw() const
        {
            return m_ctx_;
        }

        bool isValid() const
        {
            return m_refs != nullptr;
        }

        bool operator ! () const
        {
            return m_refs == nullptr;
        }

    private:
        void *m_ctx_;
        AtomicRefCounter *m_refs;
    };
}

#endif
