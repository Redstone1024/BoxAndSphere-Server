#include "ByteStreamTCP.h"

#include "../Socket.h"

#include <algorithm>

ByteStreamTCP::ByteStreamTCP(std::shared_ptr<class Socket> pSock)
	: Sock(pSock)
	, LastActiveTime(std::chrono::system_clock::now())
{
	Sock->SetNoDelay();
}

void ByteStreamTCP::Send(const std::vector<uint8_t>& Data)
{
	int32_t BytesSent;
	Sock->Send(Data.data(), Data.size(), BytesSent);
}

void ByteStreamTCP::Recv(std::vector<uint8_t>& Data, uint32_t MaxBytesRead)
{
	int32_t BytesRead;
	uint32_t PendingDataSize;
	if (Sock->HasPendingData(PendingDataSize))
	{
		PendingDataSize = std::min(PendingDataSize, MaxBytesRead);
		Data.resize(PendingDataSize);
		Sock->Recv(Data.data(), PendingDataSize, BytesRead);
		LastActiveTime = std::chrono::system_clock::now();
	}
	else Data.clear();
}

void ByteStreamTCP::Update() { }
