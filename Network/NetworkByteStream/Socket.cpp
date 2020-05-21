#include "Socket.h"	

#pragma warning(disable:4996)
#pragma warning(disable:4244)

Socket::WSAManager Socket::WSA;

Socket::Socket(HSocket pSock)
	: Sock(pSock)
{ }

Socket::Socket()
{
#ifdef _WIN32

	Sock = socket(PF_INET, SOCK_STREAM, 0);

#endif
}

Socket::~Socket()
{
#ifdef _WIN32

	Close();

#endif
}

bool Socket::Close()
{
#ifdef _WIN32

	closesocket(Sock);

	return true;

#endif

	return false;
}

bool Socket::Bind(const InternetAddr & Addr)
{
#ifdef _WIN32

	SOCKADDR_IN SocketAddr;

	SocketAddr.sin_family = PF_INET;
	SocketAddr.sin_port = htons(Addr.Port);
	SocketAddr.sin_addr.s_addr = inet_addr(Addr.IP.c_str());

	return bind(Sock, (SOCKADDR*)&SocketAddr, sizeof SocketAddr) != SOCKET_ERROR ? true : false;

#endif

	return false;
}

bool Socket::Listen(int32_t MaxBacklog)
{
#ifdef _WIN32

	return listen(Sock, MaxBacklog) != SOCKET_ERROR ? true : false;
	
#endif

	return false;
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

#endif

	return false;
}

Socket * Socket::Accept(InternetAddr & OutAddr)
{
#ifdef _WIN32

	SOCKADDR_IN ClientAddr;
	int ClientAddrLen = sizeof ClientAddr;
	SOCKET NewSocket = accept(Sock, (SOCKADDR*)&ClientAddr, &ClientAddrLen);

	if (NewSocket != INVALID_SOCKET)
	{
		OutAddr.IP = inet_ntoa(ClientAddr.sin_addr);
		OutAddr.Port = ntohs(ClientAddr.sin_port);
		return new Socket(NewSocket);
	}

	return nullptr;

#endif

	return nullptr;
}

bool Socket::Connect(InternetAddr Addr)
{
#ifdef _WIN32

	SOCKADDR_IN SocketAddr;

	SocketAddr.sin_family = PF_INET;
	SocketAddr.sin_port = htons(Addr.Port);
	SocketAddr.sin_addr.s_addr = inet_addr(Addr.IP.c_str());

	int Return = connect(Sock, (SOCKADDR*)&SocketAddr, sizeof SocketAddr);

	return ((Return == NO_ERROR) || (Return == EWOULDBLOCK) || (Return == EINPROGRESS));

#endif

	return false;
}

bool Socket::Send(const uint8_t * Data, int32_t Count, int32_t & BytesSent)
{
#ifdef _WIN32

	BytesSent = send(Sock, (const char*)Data, Count, 0);

	return BytesSent >= 0;

#endif

	return false;
}

bool Socket::Recv(uint8_t * Data, int32_t BufferSize, int32_t & BytesRead)
{
#ifdef _WIN32

	BytesRead = recv(Sock, (char*)Data, BufferSize, 0);

	return BytesRead >= 0;

#endif

	return false;
}

bool Socket::Wait(SocketWaitConditions Condition, int WaitTimeMilli)
{
#ifdef _WIN32

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

#endif

	return false;
}

SocketConnectionState Socket::GetConnectionState()
{
#ifdef _WIN32

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

#endif

	return SocketConnectionState::ConnectionError;
}

bool Socket::GetAddress(InternetAddr & OutAddr)
{
#ifdef _WIN32

	SOCKADDR_IN ThisAddr;
	int ThisAddrLen = sizeof ThisAddr;
	
	if (getsockname(Sock, (SOCKADDR*)&ThisAddr, &ThisAddrLen) == 0)
	{
		OutAddr.IP = inet_ntoa(ThisAddr.sin_addr);
		OutAddr.Port = ntohs(ThisAddr.sin_port);
		return true;
	}
	else return false;

#endif

	return false;
}

bool Socket::GetPeerAddress(InternetAddr & OutAddr)
{
#ifdef _WIN32

	SOCKADDR_IN ThisAddr;
	int ThisAddrLen = sizeof ThisAddr;

	if (getpeername(Sock, (SOCKADDR*)&ThisAddr, &ThisAddrLen) == 0)
	{
		OutAddr.IP = inet_ntoa(ThisAddr.sin_addr);
		OutAddr.Port = ntohs(ThisAddr.sin_port);
		return true;
	}
	else return false;

#endif

	return false;
}

bool Socket::SetNonBlocking(bool bIsNonBlocking)
{
#ifdef _WIN32

	u_long Value = bIsNonBlocking ? true : false;
	return ioctlsocket(Sock, FIONBIO, &Value) == 0;

#endif

	return false;
}

SocketReturn Socket::HasState(SocketStateParam State, int WaitTimeMilli)
{
#ifdef _WIN32

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

#endif

	return SocketReturn::EncounteredError;
}
