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
#include "TCPSocket.h"
#include "Queue.h"
#include "Packet.h"
#include "Thread.h"
#include "Mutex.h"
#include "Atomic.h"
#include "SignalSlot.h"

namespace m
{

	class TCPClient
	{
		M_NON_COPYABLE(TCPClient)

	public:
		TCPClient();
		~TCPClient();

		SocketConnectionError connect(const IPv4Address &addr);
		void stop();

		bool send(const FPacket &pkt)
		{
			m_sLock.lock();
			volatile bool ret = m_sQueue.offer(pkt);
			m_sLock.unlock();

			return ret;
		}

		FPacket nextPacket()
		{
			FPacket pkt;

			m_rLock.lock();
			if(!m_rQueue.isEmpty()) {
				pkt = m_rQueue.first();
				m_rQueue.poll();
			}

			m_rLock.unlock();
			return pkt;
		}

		void setConnectionTimeout(int to)
		{
			m_sock.setConnectionTimeout(to);
		}

		int connectionTimeout() const
		{
			return m_sock.connectionTimeout();
		}

		inet::SocketError lastError()
		{
			if(m_running.get() == 0)
				return m_lastError;
			else
				return m_sock.lastError();
		}

		int maxErrorCount() const
		{
			return m_maxError;
		}

		void setMaxErrorCount(int me)
		{
			if(m_running.get() == 0)
				m_maxError = me;
		}

		bool isRunning()
		{
			return m_running.get() != 0;
		}

		//Please note this signal will be called from the net thread
		//and therefore shouln't hang. If it does, packets won't be
		//received or sent until every slots returned.
		Signal<TCPClient*> onPacketAvailable;

	private:
		void threadFunc();

		TCPSocket m_sock;
		Atomic m_running;
		ClassThread<TCPClient> m_thread;

		//Error handling
		int m_maxError;
		int m_errorCount;
		inet::SocketError m_lastError;

		//Outgoing
		Mutex m_sLock;
		Queue<FPacket> m_sQueue;
		FPacket m_sPkt;
		uint32_t m_sPos;

		//Ingoing
		Mutex m_rLock;
		Queue<FPacket> m_rQueue;
		uint8_t *m_rBuffer;
		uint32_t m_rBufPos;
		PrePacket m_rPkt;
	};

}
