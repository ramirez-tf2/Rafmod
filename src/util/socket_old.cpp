#include "util/socket_old.h"


size_t FirehoseRecv_Old::Recv(size_t len, uint8_t *dst)
{
	if (!this->m_bOpened) {
		Warning("FirehoseRecv_Old: attempting to receive on non-opened socket!\n");
		return 0;
	}
	if (!this->m_bReady) {
		Warning("FirehoseRecv_Old: attempting to receive on non-ready socket!\n");
		return 0;
	}
	
#if defined _WINDOWS
	int result = recvfrom(this->m_Socket, (char *)dst, len, 0, nullptr, nullptr);
	
	if (result < 0 && WSAGetLastError() != WSAEWOULDBLOCK) {
		Warning("FirehoseRecv_Old: recvfrom failed: %d\n", WSAGetLastError());
		return 0;
	}
#else
	ssize_t result = recvfrom(this->m_Socket, dst, len, MSG_DONTWAIT, nullptr, nullptr);
	
	if (result < 0 && errno != EWOULDBLOCK) {
		Warning("FirehoseRecv_Old: recvfrom failed: %s\n", strerror(errno));
		return 0;
	}
#endif
	
	if (result < 0) {
		return 0;
	} else {
		return result;
	}
}


void FirehoseRecv_Old::Open()
{
	if (this->m_bOpened) return;
	
	this->m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (this->m_Socket < 0) {
		Warning("FirehoseRecv_Old: socket creation failed: %s\n", strerror(errno));
		return;
	}
	this->m_bOpened = true;
	
#if defined _WINDOWS
	unsigned long nonblock = 1;
	if (ioctlsocket(this->m_Socket, FIONBIO, &nonblock) == SOCKET_ERROR) {
		Warning("FirehoseRecv_Old: ioctlsocket failed: %d", WSAGetLastError());
		return;
	}
	
	sockaddr_in addr;
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port        = htons(this->m_nPort);
	
	if (bind(this->m_Socket, (const sockaddr *)&addr, sizeof(addr)) < 0) {
		Warning("FirehoseRecv_Old: bind failed: %d\n", WSAGetLastError());
		return;
	}
#endif
	
	this->m_bReady = true;
}

void FirehoseRecv_Old::Close()
{
	if (!this->m_bOpened) return;
	
#if defined _WINDOWS
	closesocket(this->m_Socket);
#else
	close(this->m_Socket);
#endif
	
	this->m_bReady  = false;
	this->m_bOpened = false;
}


void Firehose_Send(uint16_t port, size_t len, const uint8_t *src)
{
//	auto t_begin = std::chrono::high_resolution_clock::now();
	
//	for (int i = 1; i <= gpGlobals->maxClients; ++i) {
	for (int i = 1; i <= 1; ++i) {
		INetChannelInfo *info = engine->GetPlayerNetInfo(i);
		if (info == nullptr) continue;
		
		netadr_t c_addr(info->GetAddress());
		if (!c_addr.IsValid()) continue;
		
		c_addr.SetPort(port);
		
	//	DevMsg("Sending %u bytes to player %d: %s\n", len, i, c_addr.ToString());
		
		int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (s < 0) {
			Warning("Firehose_Send: socket creation failed: %s\n", strerror(errno));
			continue;
		}
		
		sockaddr_in addr;
		c_addr.ToSockadr((sockaddr *)&addr);
		
//		bool retry;
//		do {
//			retry = false;
//			
			if (sendto(s, (const char *)src, len, 0, (const sockaddr *)&addr, sizeof(addr)) < 0) {
//				if (errno == ENOBUFS) {
//					retry = true;
//				} else {
					Warning("Firehose_Send: sendto failed: %s\n", strerror(errno));
//				}
			}
//		} while (retry);
		
#if defined _WINDOWS
		closesocket(s);
#else
		close(s);
#endif
	}
	
//	auto t_end = std::chrono::high_resolution_clock::now();
	
//	auto dt = t_end - t_begin;
//	DevMsg("Firehose_Send: %lld ns\n", std::chrono::duration_cast<std::chrono::nanoseconds>(dt).count());
}

#if 0
size_t Firehose_Recv(uint16_t port, size_t len, uint8_t *dst)
{
	int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s < 0) {
		Warning("Firehose_Recv: socket creation failed: %s\n", strerror(errno));
		return 0;
	}
	
	bool fail = false;
	
#if defined _WINDOWS
	unsigned long nonblock = 1;
	if (ioctlsocket(s, FIONBIO, &nonblock) == SOCKET_ERROR) {
		Warning("Firehose_Recv: ioctlsocket call failed: %d\n", WSAGetLastError());
		return 0;
	}
	
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	
	if (bind(s, (const sockaddr *)&addr, sizeof(addr)) < 0) {
		Warning("Firehose_Recv: bind failed: %d\n", WSAGetLastError());
		closesocket(s);
		return 0;
	}
	
	int result = recvfrom(s, (char *)dst, len, 0, nullptr, nullptr);
	
	if (result < 0 && WSAGetLastError() != WSAEWOULDBLOCK) {
		Warning("Firehose_Recv: recvfrom failed: %d\n", WSAGetLastError());
	}
	
	closesocket(s);
#else
	ssize_t result = recvfrom(s, dst, len, MSG_DONTWAIT, nullptr, nullptr);
	
	if (result < 0 && errno != EWOULDBLOCK) {
		Warning("Firehose_Recv: recvfrom failed: %s\n", strerror(errno));
	}
	
	close(s);
#endif
	
	if (result >= 0) {
		return result;
	} else {
		return 0;
	}
}
#endif
