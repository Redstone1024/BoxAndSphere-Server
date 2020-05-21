#include "ConnectMakerTCP.h"

#include "../Socket.h"
#include "../ByteStream/ByteStreamTCP.h"

std::shared_ptr<ByteStream> ConnectClientMakerTCP::Construct(const std::string & IP, unsigned short Port, uint8_t Pact)
{
	std::shared_ptr<ByteStreamTCP> Result = nullptr;

	if (Pact == 0)
		Pact = 1;

	uint8_t Data;
	int32_t BytesNum;
	std::shared_ptr<Socket> Sock = std::shared_ptr<Socket>(new Socket());
	if (!Sock->Connect({ IP,Port })) return Result;
	if (!Sock->Send(&Pact, 1, BytesNum) || BytesNum != 1) return Result;

	if (!Sock->Wait(SocketWaitConditions::WaitForRead, 1000)) return Result;
	if (!Sock->Recv(&Data, 1, BytesNum) || BytesNum != 1 || Data != 'F') return Result;

	Result = std::shared_ptr<ByteStreamTCP>(new ByteStreamTCP(Sock));

	return Result;
}

std::shared_ptr<ByteStream> ConnectServerMakerTCP::Construct(std::shared_ptr<class Socket> Sock)
{
	std::shared_ptr<ByteStreamTCP> Result = nullptr;

	uint8_t Data = 'F';
	int32_t BytesNum;
	if (!Sock->Send(&Data, 1, BytesNum) || BytesNum != 1) return Result;

	Result = std::shared_ptr<ByteStreamTCP>(new ByteStreamTCP(Sock));

	return Result;
}
