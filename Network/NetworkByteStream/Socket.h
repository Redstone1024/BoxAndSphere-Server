#pragma once

#include "../../Support/Utils.h"

#include <string>
#include <memory>

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

#ifdef _WIN32

	uint64_t Sock;

	Socket(uint64_t pSock);

#endif  // _WIN32

#ifdef __linux__

	int Sock;

	Socket(int pSock);

#endif  // __linux__


public:

	Socket();

	~Socket();

	bool Close();

	bool Shutdown();

	bool Bind(const InternetAddr& Addr);

	bool Listen(int32_t MaxBacklog);

	bool SetNoDelay(bool bIsNoDelay = true);

	bool HasPendingData(uint32_t& PendingDataSize);

	class Socket* Accept(InternetAddr& OutAddr);

	bool Connect(InternetAddr Addr);

	bool Send(const uint8_t* Data, int32_t Count, int32_t& BytesSent);

	bool Recv(uint8_t* Data, int32_t BufferSize, int32_t& BytesRead);

	bool Wait(SocketWaitConditions Condition, int WaitTimeMilli);

	SocketConnectionState GetConnectionState();

	bool GetAddress(InternetAddr& OutAddr);

	bool GetPeerAddress(InternetAddr& OutAddr);

	SocketReturn HasState(SocketStateParam State, int WaitTimeMilli = 0);
};
