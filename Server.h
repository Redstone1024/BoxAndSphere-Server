#pragma once

#include "Support/Utils.h"

#include "Support/Arguments.h"
#include "ServerSetting.h"

#include <thread>
#include <queue>
#include <mutex>

class Server
{
private:
	Arguments ServerArguments;

	ServerSetting Setting;

	std::shared_ptr<class Socket> MainSocket;
	std::shared_ptr<std::thread> SocketThread;

	uint64_t NowTickNum;
	std::mutex EventQueueMutex;
	std::queue<std::vector<uint8_t>> EventQueue;

	uint16_t NowPlayerID;
	std::vector<std::shared_ptr<class Player>> Players;

	bool Stopping;

public:

	Server(Arguments Arg);

	~Server();

	void Run();

	void Stop();

public:

	// 服务器监听Socket线程
	void SocketFunction();

	// 玩家线程
	void PlayerFunction(std::weak_ptr<class Player> Owner);

};
