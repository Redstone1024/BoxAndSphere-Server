#pragma once

#include "Network/NetworkByteStream/Socket.h"

struct ServerSetting
{
	InternetAddr NetAddr;

	unsigned int FPS;

	std::string Description;

};
