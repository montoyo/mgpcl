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

#include "mgpcl/TCPServer.h"

#define M_TCPSERVER_BUFSZ 8192

m::TCPServer::TCPServer() : m_thread("TCP-Server")
{
	m_maxError = 16;
	m_errorCount = 0;
	m_lastError = inet::kSE_NoError;
	m_backlog = SOMAXCONN;
}

m::TCPServer::~TCPServer()
{
	stop();
}

bool m::TCPServer::listen(uint16_t port)
{
	IPv4Address any(port);
	return listen(any);
}

bool m::TCPServer::listen(const IPv4Address &addr)
{
	if(!m_sock.initialize()) {
		m_lastError = m_sock.lastError();
		return false;
	}

	if(!m_sock.bind(addr)) {
		m_lastError = m_sock.lastError();
		return false;
	}

	if(!m_sock.listen(m_backlog)) {
		m_lastError = m_sock.lastError();
		return false;
	}

	m_running.set(1);
	m_thread.setFunc(this, &TCPServer::threadFunc);
	m_thread.start();
	return true;
}

void m::TCPServer::stop()
{
	if(m_running.get()) {
		m_running.set(0);
		m_thread.join();

		for(TCPServerClient *cli : m_clients) {
			closesocket(cli->m_sock);
			delete cli;
		}

		m_clients.clear();
		m_sock.close();
	}
}

void m::TCPServer::threadFunc()
{
	while(m_running.get()) {
		fd_set rdSet, wrSet;
		FD_ZERO(&rdSet);
		FD_ZERO(&wrSet);

		SOCKET smax = m_sock.raw();
		FD_SET(m_sock.raw(), &rdSet); //To accept new clients

		m_clLock.lockFor(RWAction::Reading);
		for(TCPServerClient *cli : m_clients) {
			SOCKET s = cli->m_sock;
			if(s > smax)
				smax = s;

			FD_SET(s, &rdSet);
			if(cli->needsWrite())
				FD_SET(s, &wrSet);
		}
		m_clLock.releaseFor(RWAction::Reading);

		struct timeval tv;
		inet::fillTimeval(tv, 10);

		int ret = select(smax + 1, &rdSet, &wrSet, nullptr, &tv);
		if(ret > 0) {
			if(FD_ISSET(m_sock.raw(), &rdSet)) {
				//Incoming client; accept him...
				IPv4Address addr;
				socklen_t addrSz = addr.rawSize();

				SOCKET sock = accept(m_sock.raw(), reinterpret_cast<struct sockaddr*>(addr.raw()), &addrSz);
				if(sock != INVALID_SOCKET) {
					if(addrSz != addr.rawSize())
						closesocket(sock);
					else {
						TCPServerClient *cli = new TCPServerClient(this, addr, sock);

						m_clLock.lockFor(RWAction::Writing);
						m_clients.add(cli);
						m_clLock.releaseFor(RWAction::Writing);

						onClientConnected(cli);
					}
				}
			}

			int needsDelete = 0;
			m_clLock.lockFor(RWAction::Reading);
			for(TCPServerClient *cli : m_clients) {
				bool status = false;

				if(FD_ISSET(cli->m_sock, &rdSet))
					status = cli->readyRead();

				if(!status && FD_ISSET(cli->m_sock, &wrSet))
					status = cli->readyWrite();

				if(status)
					needsDelete++;
			}
			m_clLock.releaseFor(RWAction::Reading);

			if(needsDelete > 0) {
				//Close erroring/disconnected clients
				int found = 0;
				TCPServerClient **removed;

				if(needsDelete <= 4)
					removed = m_tmp;
				else
					removed = new TCPServerClient*[needsDelete];

				m_clLock.lockFor(RWAction::Writing);
				for(int i = m_clients.size() - 1; i >= 0 && found < needsDelete; i--) {
					TCPServerClient *cli = m_clients[i];

					if(cli->m_disconnected || cli->m_errorCount >= m_maxError) {
						closesocket(cli->m_sock);
						m_clients.remove(i);
						removed[found++] = cli;
					}
				}
				m_clLock.releaseFor(RWAction::Writing);

				for(int i = 0; i < found; i++) {
					onClientDisconnected(removed[i], removed[i]->m_disconnected);
					delete removed[i];
				}

				if(needsDelete > 4)
					delete[] removed;
			}
		} else if(ret != 0) {
			m_lastError = inet::kSE_UnknownError;

			if(++m_errorCount >= m_maxError) {
				m_running.set(0);

				m_clLock.lockFor(RWAction::Writing);
				for(TCPServerClient *cli : m_clients) {
					closesocket(cli->m_sock);
					delete cli;
				}

				m_clients.clear();
				m_clLock.releaseFor(RWAction::Writing);

				m_sock.close();
			}
		}
	}
}

m::TCPServerClient::TCPServerClient(TCPServer *p, const IPv4Address &addr, SOCKET s) : m_address(addr), m_rQueue(128), m_sQueue(128)
{
	m_parent = p;
	m_sock = s;
	m_errorCount = 0;
	m_disconnected = false;
	m_lastError = inet::kSE_NoError;
	m_userdata = nullptr;
	m_sPos = 0;
	m_rBuf = new uint8_t[M_TCPSERVER_BUFSZ];
	m_rPos = 0;
}

m::TCPServerClient::~TCPServerClient()
{
	delete[] m_rBuf;
}

bool m::TCPServerClient::needsWrite()
{
	if(m_sPkt.isValid())
		return true;

	bool ret = false;
	m_sLock.lock();

	if(!m_sQueue.isEmpty()) {
		m_sPkt = m_sQueue.first();
		m_sQueue.poll();
		ret = true;
	}

	m_sLock.unlock();
	return ret;
}

bool m::TCPServerClient::readyRead()
{
	int rd = recv(m_sock, reinterpret_cast<char*>(m_rBuf) + m_rPos, static_cast<int>(M_TCPSERVER_BUFSZ - m_rPos), 0);
	if(rd > 0) {
		uint32_t avail = static_cast<uint32_t>(rd) + m_rPos;
		uint8_t *ptr = m_rBuf;
		m_rPos = 0;

		if(m_rPkt.isValid()) {
			uint32_t added = m_rPkt.fill(ptr, avail);
			avail -= added;
			ptr += added;

			if(m_rPkt.isReady()) {
				m_rLock.lock();
				m_rQueue.offer(m_rPkt.finalize());
				m_rLock.unlock();

				m_parent->onPacketAvailable(this);
			}
		}

		while(avail > 0) {
			if(avail < sizeof(uint32_t)) {
				m_rPos = avail;
				if(ptr != m_rBuf)
					Mem::move(m_rBuf, ptr, avail);

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

						m_parent->onPacketAvailable(this);
					}
				}
			}
		}
	} else if(rd == 0) {
		m_disconnected = true;
		return true;
	} else {
		//Error!
		m_lastError = inet::socketError();

		if(m_lastError != inet::kSE_WouldBlock && ++m_errorCount >= m_parent->maxError())
			return true;
	}

	return false;
}

bool m::TCPServerClient::readyWrite()
{
	if(m_sPkt.isValid()) {
		int ret = ::send(m_sock, reinterpret_cast<const char*>(m_sPkt.data() + m_sPos), static_cast<int>(m_sPkt.size() - m_sPos), 0);

		if(ret > 0) {
			m_sPos += static_cast<uint32_t>(ret);

			if(m_sPos >= m_sPkt.size()) {
				m_sPkt.destroy();
				m_sPos = 0;
			}
		} else if(ret == 0) {
			m_disconnected = true;
			return true;
		} else {
			//Error!
			m_lastError = inet::socketError();

			if(m_lastError != inet::kSE_WouldBlock && ++m_errorCount >= m_parent->maxError())
				return true;
		}
	}

	return false;
}
