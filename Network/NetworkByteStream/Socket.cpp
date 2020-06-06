#include "Socket.h"	

#ifdef _WIN32

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#pragma warning(disable:4996)
#pragma warning(disable:4244)

#endif  // _WIN32

#ifdef __linux__

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#define SOCKET_ERROR -1
#define INVALID_SOCKET 0

#endif  // __linux__

#ifdef _WIN32

struct WSAManager
{
	WSADATA Data;
	WSAManager() { WSAStartup(MAKEWORD(2, 2), &Data); }
	~WSAManager() { WSACleanup(); }
	};

WSAManager WSA;

#endif  // _WIN32
#ifdef _WIN32

Socket::Socket(uint64_t pSock)
	: Sock(pSock)
{ }

#endif  // _WIN32

#ifdef __linux__

Socket::Socket(int pSock)
	: Sock(pSock)
{ }

#endif  // __linux__

Socket::Socket()
{
	Sock = socket(PF_INET, SOCK_STREAM, 0);
}

Socket::~Socket()
{
	Close();
}

bool Socket::Close()
{
#ifdef _WIN32

	closesocket(Sock);

	return true;

#endif  // _WIN32

#ifdef __linux__

	close(Sock);

	return true;

#endif // __linux__

	return false;
}

bool Socket::Shutdown()
{
#ifdef _WIN32

	shutdown(Sock, SD_BOTH);

	return true;

#endif  // _WIN32

#ifdef __linux__

	shutdown(Sock, SHUT_RDWR);

	return true;

#endif // __linux__

	return false;
}

bool Socket::Bind(const InternetAddr & Addr)
{
	sockaddr_in SocketAddr;

	SocketAddr.sin_family = PF_INET;
	SocketAddr.sin_port = htons(Addr.Port);
	if (Addr.IP == "Default") SocketAddr.sin_addr.s_addr = INADDR_ANY;
	else SocketAddr.sin_addr.s_addr = inet_addr(Addr.IP.c_str());

	return bind(Sock, (sockaddr*)&SocketAddr, sizeof SocketAddr) != SOCKET_ERROR ? true : false;
}

bool Socket::Listen(int32_t MaxBacklog)
{
	return listen(Sock, MaxBacklog) != SOCKET_ERROR ? true : false;
}

bool Socket::SetNoDelay(bool bIsNoDelay)
{
	int Param = bIsNoDelay ? 1 : 0;
	return setsockopt(Sock, IPPROTO_TCP, TCP_NODELAY, (char*)&Param, sizeof(Param)) == 0;
}

bool Socket::HasPendingData(uint32_t & PendingDataSize)
{
#ifdef _WIN32

	PendingDataSize = 0;

	if (HasState(SocketStateParam::CanRead) == SocketReturn::Yes)
	{
		if (ioctlsocket(Sock, FIONREAD, (u_long*)(&PendingDataSize)) == 0)
		{
			return (PendingDataSize > 0);
		}
	}

	return false;

#endif  // _WIN32

#ifdef __linux__

	PendingDataSize = 0;

	if (HasState(SocketStateParam::CanRead) == SocketReturn::Yes)
	{
		if (ioctl(Sock, FIONREAD, (u_long*)(&PendingDataSize)) == 0)
		{
			return (PendingDataSize > 0);
		}
	}

	return false;

#endif // __linux__

	return false;
}

Socket * Socket::Accept(InternetAddr & OutAddr)
{
	sockaddr_in ClientAddr;

#ifdef _WIN32

	int ClientAddrLen = sizeof ClientAddr;

#endif  // _WIN32

#ifdef __linux__

	socklen_t ClientAddrLen = sizeof ClientAddr;

#endif // __linux__
	
	auto NewSocket = accept(Sock, (sockaddr*)&ClientAddr, &ClientAddrLen);

	if (NewSocket != INVALID_SOCKET)
	{
		OutAddr.IP = inet_ntoa(ClientAddr.sin_addr);
		OutAddr.Port = ntohs(ClientAddr.sin_port);
		return new Socket(NewSocket);
	}

	return nullptr;
}

bool Socket::Connect(InternetAddr Addr)
{
	sockaddr_in SocketAddr;

	SocketAddr.sin_family = PF_INET;
	SocketAddr.sin_port = htons(Addr.Port);
	SocketAddr.sin_addr.s_addr = inet_addr(Addr.IP.c_str());

	int Return = connect(Sock, (sockaddr*)&SocketAddr, sizeof SocketAddr);

#ifdef _WIN32

	return ((Return == NO_ERROR) || (Return == EWOULDBLOCK) || (Return == EINPROGRESS));

#endif  // _WIN32

#ifdef __linux__

	return Return == 0;

#endif // __linux__

	return false;
}

bool Socket::Send(const uint8_t * Data, int32_t Count, int32_t & BytesSent)
{
#ifdef _WIN32

	BytesSent = send(Sock, (const char*)Data, Count, 0);

#endif  // _WIN32

#ifdef __linux__

	BytesSent = send(Sock, (const char*)Data, Count, MSG_NOSIGNAL);

#endif // __linux__

	return BytesSent >= 0;
}

bool Socket::Recv(uint8_t * Data, int32_t BufferSize, int32_t & BytesRead)
{
	BytesRead = recv(Sock, (char*)Data, BufferSize, 0);

	return BytesRead >= 0;
}

bool Socket::Wait(SocketWaitConditions Condition, int WaitTimeMilli)
{
	if ((Condition == SocketWaitConditions::WaitForRead) || (Condition == SocketWaitConditions::WaitForReadOrWrite))
	{
		if (HasState(SocketStateParam::CanRead, WaitTimeMilli) == SocketReturn::Yes)
		{
			return true;
		}
	}

	if ((Condition == SocketWaitConditions::WaitForWrite) || (Condition == SocketWaitConditions::WaitForReadOrWrite))
	{
		if (HasState(SocketStateParam::CanWrite, WaitTimeMilli) == SocketReturn::Yes)
		{
			return true;
		}
	}

	return false;
}

SocketConnectionState Socket::GetConnectionState()
{
	SocketConnectionState CurrentState = SocketConnectionState::ConnectionError;

	if (HasState(SocketStateParam::HasError) == SocketReturn::No)
	{
		SocketReturn WriteState = HasState(SocketStateParam::CanWrite, 1);
		SocketReturn ReadState = HasState(SocketStateParam::CanRead, 1);

		if (WriteState == SocketReturn::Yes || ReadState == SocketReturn::Yes)
			CurrentState = SocketConnectionState::Connected;
		else if (WriteState == SocketReturn::No && ReadState == SocketReturn::No)
			CurrentState = SocketConnectionState::NotConnected;

	}

	return CurrentState;
}

bool Socket::GetAddress(InternetAddr & OutAddr)
{
	sockaddr_in ThisAddr;

#ifdef _WIN32

	int ThisAddrLen = sizeof ThisAddr;

#endif  // _WIN32

#ifdef __linux__

	socklen_t ThisAddrLen = sizeof ThisAddr;

#endif // __linux__
	
	if (getsockname(Sock, (sockaddr*)&ThisAddr, &ThisAddrLen) == 0)
	{
		OutAddr.IP = inet_ntoa(ThisAddr.sin_addr);
		OutAddr.Port = ntohs(ThisAddr.sin_port);
		return true;
	}
	else return false;
}

bool Socket::GetPeerAddress(InternetAddr & OutAddr)
{
	sockaddr_in ThisAddr;

#ifdef _WIN32

	int ThisAddrLen = sizeof ThisAddr;

#endif  // _WIN32

#ifdef __linux__

	socklen_t ThisAddrLen = sizeof ThisAddr;

#endif // __linux__

	if (getpeername(Sock, (sockaddr*)&ThisAddr, &ThisAddrLen) == 0)
	{
		OutAddr.IP = inet_ntoa(ThisAddr.sin_addr);
		OutAddr.Port = ntohs(ThisAddr.sin_port);
		return true;
	}
	else return false;
}

SocketReturn Socket::HasState(SocketStateParam State, int WaitTimeMilli)
{
	timeval Time;
	Time.tv_sec = WaitTimeMilli / 1000;
	Time.tv_usec = (WaitTimeMilli % 1000) * 1000;

	fd_set SocketSet;

	FD_ZERO(&SocketSet);
	FD_SET(Sock, &SocketSet);

	timeval* TimePointer = WaitTimeMilli >= 0 ? &Time : nullptr;

	int32_t SelectStatus = 0;
	switch (State)
	{
	case SocketStateParam::CanRead:
		SelectStatus = select(Sock + 1, &SocketSet, NULL, NULL, TimePointer);
		break;

	case SocketStateParam::CanWrite:
		SelectStatus = select(Sock + 1, NULL, &SocketSet, NULL, TimePointer);
		break;

	case SocketStateParam::HasError:
		SelectStatus = select(Sock + 1, NULL, NULL, &SocketSet, TimePointer);
		break;
	}

	return SelectStatus > 0 ? SocketReturn::Yes :
		SelectStatus == 0 ? SocketReturn::No :
		SocketReturn::EncounteredError;
}
