#pragma once

#include "../../../Support/Utils.h"

#include "../ConnectMaker.h"

class ConnectClientMakerTCP : public ConnectClientMaker
{
public:
	virtual std::shared_ptr<ByteStream> Construct(const std::string& IP, unsigned short Port, uint8_t Pact = 1) final;
};

class ConnectServerMakerTCP : public ConnectServerMaker
{
public:
	virtual uint8_t GetPactID() final { return 1; }
	virtual std::shared_ptr<ByteStream> Construct(std::shared_ptr<class Socket> Sock) final;
};
