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

#include "mgpcl/TCPClient.h"

#define M_TCPCLIENT_BUFSZ 8192

m::TCPClient::TCPClient() : m_thread("TCP-Client"), m_sQueue(128), m_rQueue(128)
{
    m_sPos = 0;
    m_rBufPos = 0;
    m_rBuffer = new uint8_t[M_TCPCLIENT_BUFSZ];
    m_errorCount = 0;
    m_maxError = 16;
    m_lastError = inet::kSE_NoError;
}

m::TCPClient::~TCPClient()
{
    stop();
    delete[] m_rBuffer;
}

m::SocketConnectionError m::TCPClient::connect(const IPv4Address &addr)
{
    if(!m_sock.initialize())
        return kSCE_UnknownError;

    SocketConnectionError err = m_sock.connect(addr);
    if(err != kSCE_NoError)
        return err;

    m_running.set(1);
    m_thread.setFunc(this, &TCPClient::threadFunc);
    if(!m_thread.start())
        return kSCE_UnknownError;

    return kSCE_NoError;
}

void m::TCPClient::threadFunc()
{
    while(m_running.get()) {
        fd_set rdSet, wrSet;
        FD_ZERO(&rdSet);
        FD_ZERO(&wrSet);
        FD_SET(m_sock.raw(), &rdSet);
        FD_SET(m_sock.raw(), &wrSet);

        struct timeval tv;
        inet::fillTimeval(tv, 10);

        bool doOut;
        if(m_sPkt.isValid())
            doOut = true;
        else {
            m_sLock.lock();

            if(m_sQueue.isEmpty())
                doOut = false;
            else {
                m_sPkt = m_sQueue.first();
                m_sQueue.poll();

                doOut = true;
            }

            m_sLock.unlock();
        }

        inet::SocketError err = inet::kSE_NoError;
        int ret = select(m_sock.raw() + 1, &rdSet, doOut ? &wrSet : nullptr, nullptr, &tv);

        if(ret > 0) {
            if(FD_ISSET(m_sock.raw(), &rdSet)) {
                //Ready to read some data
                ret = recv(m_sock.raw(), reinterpret_cast<char*>(m_rBuffer + m_rBufPos), static_cast<int>(M_TCPCLIENT_BUFSZ - m_rBufPos), 0);

                if(ret > 0) {
                    uint32_t avail = static_cast<uint32_t>(ret) + m_rBufPos;
                    uint8_t *ptr = m_rBuffer;
                    m_rBufPos = 0;

                    if(m_rPkt.isValid()) {
                        uint32_t added = m_rPkt.fill(ptr, avail);
                        avail -= added;
                        ptr += added;

                        if(m_rPkt.isReady()) {
                            m_rLock.lock();
                            m_rQueue.offer(m_rPkt.finalize());
                            m_rLock.unlock();

                            onPacketAvailable(this);
                        }
                    }

                    while(avail > 0) {
                        if(avail < sizeof(uint32_t)) {
                            m_rBufPos = avail;
                            if(ptr != m_rBuffer)
                                mem::move(m_rBuffer, ptr, avail);

                            //Come back later
                            break;
                        }

                        uint32_t pktSize = *reinterpret_cast<uint32_t*>(ptr);
                        pktSize = ((pktSize & 0x000000FF) << 24) | ((pktSize & 0x0000FF00) << 8) | ((pktSize & 0x00FF0000) >> 8) | ((pktSize & 0xFF000000) >> 24);
                        avail -= sizeof(uint32_t);
                        ptr += sizeof(uint32_t);

                        if(pktSize > sizeof(uint32_t)) { //Packets should always be bigger than 4 bytes
                            m_rPkt.setSize(pktSize - sizeof(uint32_t));

                            if(avail > 0) {
                                uint32_t added = m_rPkt.fill(ptr, avail);
                                avail -= added;
                                ptr += added;

                                if(m_rPkt.isReady()) {
                                    m_rLock.lock();
                                    m_rQueue.offer(m_rPkt.finalize());
                                    m_rLock.unlock();

                                    onPacketAvailable(this);
                                }
                            }
                        }
                    }
                } else if(ret == 0) {
                    //Connection closed
                    m_running.set(0);
                    m_sock.close();
                    return;
                } else
                    err = inet::socketError();
            }

            if(doOut && FD_ISSET(m_sock.raw(), &wrSet)) {
                //Ready to write some data
                ret = ::send(m_sock.raw(), reinterpret_cast<const char*>(m_sPkt.data() + m_sPos), static_cast<int>(m_sPkt.size() - m_sPos), 0);

                if(ret > 0) {
                    m_sPos += static_cast<uint32_t>(ret);

                    if(m_sPos >= m_sPkt.size()) { //Everything was sent, we can destroy the packet
                        m_sPkt.destroy();
                        m_sPos = 0;
                    }
                } else if(ret == 0) {
                    //Connection closed
                    m_running.set(0);
                    m_sock.close();
                    return;
                } else
                    err = inet::socketError();
            }
        } else if(ret != 0) {
            //select() failed
            err = inet::kSE_UnknownError;
        }

        if(err != inet::kSE_NoError && err != inet::kSE_WouldBlock) {
            if(++m_errorCount >= m_maxError) {
                m_running.set(0);
                m_lastError = err;
                m_sock.close();
                return;
            }
        } else
            m_errorCount = 0; //no more errors, reset error count
    }
}

void m::TCPClient::stop()
{
    if(m_running.get()) {
        m_running.set(0);
        m_thread.join();
        m_sock.close();
    }
}
