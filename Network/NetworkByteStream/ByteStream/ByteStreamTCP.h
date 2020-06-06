#pragma once

#include "../../../Support/Utils.h"

#include "../ByteStream.h"

#include <memory>

class ByteStreamTCP : public ByteStream
{
	friend class ConnectClientMakerTCP;
	friend class ConnectServerMakerTCP;

private:
	std::shared_ptr<class Socket> Sock;
	std::chrono::system_clock::time_point LastActiveTime;

	ByteStreamTCP(std::shared_ptr<class Socket> pSock)
		: Sock(pSock)
		, LastActiveTime(std::chrono::system_clock::now())
	{ }

public:
	virtual ~ByteStreamTCP() final { }

	virtual void Send(const std::vector<uint8_t>& Data) final;
	virtual void Recv(std::vector<uint8_t>& Data) final;
	virtual void Update() final;

	virtual std::chrono::system_clock::time_point GetLastActiveTime() final { return LastActiveTime; }
};
