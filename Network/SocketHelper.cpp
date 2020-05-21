#include "SocketHelper.h"

#include "NetworkByteStream/Socket.h"

std::vector<uint8_t> SocketHelper::StringToArray(const std::string & A)
{
	std::vector<uint8_t> Bytes;
	Bytes.resize(A.length() + 1);
	for (int i = 0; i < A.length(); i++) Bytes[i] = A[i];
	return Bytes;
}

std::string SocketHelper::ArrayToString(const std::vector<uint8_t>& A)
{
	return std::string((char*)A.data());
}

bool SocketHelper::SendWithTimeout(std::weak_ptr<Socket> Sock, const std::vector<uint8_t>& Data, int32_t & BytesSent, int WaitTimeMilli)
{
	if (std::shared_ptr<Socket> tSocket = Sock.lock())
	{
		if (tSocket->Wait(SocketWaitConditions::WaitForWrite, WaitTimeMilli))
		{
			std::vector<uint8_t> BufferSend = Data;
			BufferSend.resize(BufferSize);
			return tSocket->Send(BufferSend.data(), BufferSize, BytesSent);
		}
	}

	return false;
}

bool SocketHelper::RecvWithTimeout(std::weak_ptr<Socket> Sock, std::vector<uint8_t>& Data, int32_t & BytesRead, int WaitTimeMilli)
{
	if (std::shared_ptr<Socket> tSocket = Sock.lock())
	{
		if (tSocket->Wait(SocketWaitConditions::WaitForRead, WaitTimeMilli))
		{
			Data.resize(BufferSize);
			return tSocket->Recv(Data.data(), BufferSize, BytesRead);
		}
	}

	return false;
}

bool SocketHelper::ClientRequest(std::weak_ptr<Socket> Sock, const LockstepPassInfo & Info, int WaitTimeMilli)
{
	if (std::shared_ptr<Socket> tSocket = Sock.lock())
	{
		int32_t BytesNum = 0;
		std::vector<uint8_t> BufferRecv;
		if (RecvWithTimeout(Sock, BufferRecv, BytesNum, WaitTimeMilli))
		{
			if (BytesNum == BufferSize && ArrayToString(BufferRecv) == Info.Pass)
			{
				return SendWithTimeout(Sock, StringToArray(Info.Send), BytesNum, WaitTimeMilli);
			}
		}
	}

	return false;
}
