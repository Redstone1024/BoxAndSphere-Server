#pragma once

#include "Support/Utils.h"

#include <thread>
#include <queue>

struct Player
{
	uint16_t ID;
	std::string Description;

	std::shared_ptr<std::thread> Thread;
	std::shared_ptr<class Socket> OwnSocket;

	std::mutex EventQueueMutex;
	std::queue<std::queue<std::vector<uint8_t>>> EventQueue;

	bool Stopping = true;

};
