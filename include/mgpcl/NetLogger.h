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
#include "TCPClient.h"
#include "Logger.h"
#include "ReadWriteLock.h"

namespace m
{
    class NetLogger : public Logger, public SlotCapable
    {
        M_NON_COPYABLE(NetLogger)

    public:
        NetLogger()
        {
            m_connErr = kSCE_NoError;
            m_filter = ~uint32_t(0);
            m_attempts = 3;

            m_cli.setConnectionTimeout(1000);
            m_cli.onPacketAvailable.connect(this, &NetLogger::onPacketReceived);
        }

        NetLogger(const String &addr)
        {
            m_attempts = 3;
            m_connErr = kSCE_NoError;
            m_filter = ~uint32_t(0);

            m_cli.setConnectionTimeout(1000);
            m_cli.onPacketAvailable.connect(this, &NetLogger::onPacketReceived);
            connect(addr);
        }

        void vlog(LogLevel level, const char *fname, int line, const char *format, VAList *lst) override;
        bool connect(const String &ip);

        bool isEnabled(LogLevel lvl)
        {
            m_lock.lockFor(RWAction::Reading);
            volatile uint32_t filter = m_filter;
            m_lock.releaseFor(RWAction::Reading);

            return (filter & (1 << static_cast<uint32_t>(lvl))) != 0;
        }

        void disconnect()
        {
            m_cli.stop();
        }

        bool isRunning()
        {
            return m_cli.isRunning();
        }

        inet::SocketError lastError()
        {
            return m_cli.lastError();
        }

        SocketConnectionError connectionError() const
        {
            return m_connErr;
        }

        bool tryAutoStartSubprocess(bool cod = true) const;

        int connectionTimeout() const
        {
            return m_cli.connectionTimeout();
        }

        void setConnectionTimeout(int to)
        {
            m_cli.setConnectionTimeout(to);
        }

        int maxConnectionAttempts() const
        {
            return m_attempts;
        }

        void setMaxConnectionAttempts(int a)
        {
            m_attempts = a;
        }

    private:
        TCPClient m_cli;
        SocketConnectionError m_connErr;

        bool startSubprocess(const String &jar, bool cod) const;
        bool onPacketReceived(TCPClient *cli);

        int m_attempts;
        ReadWriteLock m_lock;
        volatile uint32_t m_filter;
    };
}
