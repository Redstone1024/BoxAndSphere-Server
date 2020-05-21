#pragma once

#include "../../Support/Utils.h"

#include <string>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET HSocket;
#endif

#ifdef linux
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef int HSocket;
#define SOCKET_ERROR  (-1)
#define INVALID_SOCKET  0
#endif
#include <iostream>

struct InternetAddr
{
	std::string IP;
	unsigned short Port;
};

enum class SocketWaitConditions
{
	WaitForRead,
	WaitForWrite,
	WaitForReadOrWrite
};

enum class SocketConnectionState
{
	NotConnected,
	Connected,
	ConnectionError
};

enum class SocketReturn : uint8_t
{
	Yes,
	No,
	EncounteredError,
};

enum class SocketStateParam : uint8_t
{
	CanRead,
	CanWrite,
	HasError,
};

class Socket
{
private:
	HSocket Sock;

#ifdef _WIN32
	
	struct WSAManager
	{
		WSADATA Data;
		WSAManager() { WSAStartup(MAKEWORD(2, 2), &Data); }
		~WSAManager() { WSACleanup(); }
	};

	static WSAManager WSA;

#endif

	Socket(HSocket pSock);

public:

	Socket();

	~Socket();

	bool Close();

	bool Bind(const InternetAddr& Addr);

	bool Listen(int32_t MaxBacklog);

	bool HasPendingData(uint32_t& PendingDataSize);

	class Socket* Accept(InternetAddr& OutAddr);

	bool Connect(InternetAddr Addr);

	bool Send(const uint8_t* Data, int32_t Count, int32_t& BytesSent);

	bool Recv(uint8_t* Data, int32_t BufferSize, int32_t& BytesRead);

	bool Wait(SocketWaitConditions Condition, int WaitTimeMilli);

	SocketConnectionState GetConnectionState();

	bool GetAddress(InternetAddr& OutAddr);

	bool GetPeerAddress(InternetAddr& OutAddr);

	bool SetNonBlocking(bool bIsNonBlocking = true);

	SocketReturn HasState(SocketStateParam State, int WaitTimeMilli = 0);
};
