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
#include "Thread.h"
#include "Atomic.h"
#include "ReadWriteLock.h"
#include "List.h"
#include "Queue.h"
#include "Packet.h"
#include "SignalSlot.h"

namespace m
{
	class TCPServer;

	class TCPServerClient
	{
		friend class TCPServer;
		M_NON_COPYABLE(TCPServerClient)

	public:
		void setUserdata(void *ud)
		{
			m_userdata = ud;
		}

		void *userdata() const
		{
			return m_userdata;
		}

		TCPServer *server() const
		{
			return m_parent;
		}

		inet::SocketError lastError() const
		{
			return m_lastError;
		}

		const IPv4Address &address() const
		{
			return m_address;
		}

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

	private:
		TCPServerClient()
		{
		}

		TCPServerClient(TCPServer *p, const IPv4Address &addr, SOCKET s);
		~TCPServerClient();

		bool needsWrite();
		bool readyRead();
		bool readyWrite();

		TCPServer *m_parent;
		SOCKET m_sock;
		IPv4Address m_address;
		int m_errorCount;
		bool m_disconnected;
		inet::SocketError m_lastError;
		void *m_userdata;

		//Out
		Mutex m_sLock;
		Queue<FPacket> m_sQueue;
		FPacket m_sPkt;
		uint32_t m_sPos;

		//In
		Mutex m_rLock;
		Queue<FPacket> m_rQueue;
		uint8_t *m_rBuf;
		uint32_t m_rPos;
		PrePacket m_rPkt;
	};

	class TCPServer
	{
		M_NON_COPYABLE(TCPServer)

	public:
		TCPServer();
		~TCPServer();

		bool listen(uint16_t port);
		bool listen(const IPv4Address &addr);
		void stop();

		bool running()
		{
			return m_running.get() != 0;
		}

		int backlog() const
		{
			return m_backlog;
		}

		inet::SocketError lastError() const
		{
			return m_lastError;
		}

		void setBacklog(int bl = SOMAXCONN)
		{
			//Can only be changed before listen() is called.
			m_backlog = bl;
		}

		int maxError() const
		{
			return m_maxError;
		}

		void setMaxError(int me)
		{
			m_maxError = me;
		}

		int numClients()
		{
			m_clLock.lockFor(RWAction::Reading);
			volatile int ret = m_clients.size();
			m_clLock.releaseFor(RWAction::Reading);

			return ret;
		}

		TCPServerClient *client(int idx)
		{
			m_clLock.lockFor(RWAction::Reading);
			TCPServerClient *volatile ret = m_clients[idx];
			m_clLock.releaseFor(RWAction::Reading);

			return ret;
		}

		Signal<TCPServerClient*> onClientConnected;
		Signal<TCPServerClient*> onPacketAvailable;
		Signal<TCPServerClient*, bool> onClientDisconnected; //The boolean indicates if the connection was closed gently.

	private:
		void threadFunc();

		TCPSocket m_sock;
		Atomic m_running;
		ClassThread<TCPServer> m_thread;

		//Error handling
		int m_maxError;
		int m_errorCount;
		inet::SocketError m_lastError;

		//Client handling
		int m_backlog;
		ReadWriteLock m_clLock;
		List<TCPServerClient*> m_clients;
		TCPServerClient *m_tmp[4];
	};
}
