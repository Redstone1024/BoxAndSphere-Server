#include "Server.h"

#include "Network/NetworkByteStream/Socket.h"
#include "Support/Log.h"
#include "Network/SocketHelper.h"
#include "Player.h"

#include <chrono>

Server::Server(Arguments Arg)
	: ServerArguments(Arg)
	, Stopping(false)
	, NowPlayerID(1)
	, NowTickNum(0)
{
	// 初始化服务器设置参数
	Setting.NetAddr.IP = ServerArguments.GetValue("IP", "127.0.0.1");
	Setting.NetAddr.Port = std::stoi(ServerArguments.GetValue("Port", "25567"));
	Setting.FPS = std::stoi(ServerArguments.GetValue("FPS", "40"));
	Setting.Description = ServerArguments.GetValue("Description", "Default");
}

Server::~Server()
{
	if (MainSocket)
		MainSocket = nullptr;

	Stopping = true;

	if (SocketThread && SocketThread->joinable())
		SocketThread->join();
	SocketThread = nullptr;

	for (auto x : Players)
	{
		x->Stopping = true;

		if (x->Thread && x->Thread->joinable())
			x->Thread->join();
		x->Thread = nullptr;
	}

}

void Server::Run()
{
	// 初始化Log通道
	Log::AddChannel("Main", "Main", true);
	Log::AddChannel("Socket", "Socket", true);

	Log::Write("Main", "Initialize Complete!");

	MainSocket = std::shared_ptr<Socket>(new Socket());
	if (!MainSocket->Bind(Setting.NetAddr)) throw "Socket Bind Fail!";
	Log::Write("Socket", "Main Socket Bind Complete!");

	SocketThread = std::shared_ptr<std::thread>(new std::thread(&Server::SocketFunction, this));

	std::pair<std::chrono::high_resolution_clock::time_point, std::chrono::high_resolution_clock::time_point> Timer;
	std::chrono::nanoseconds TheoryDifferenceTime(std::chrono::nanoseconds(std::chrono::seconds(1)) / Setting.FPS);
	std::chrono::nanoseconds RealDifferenceTime;
	while (!Stopping)
	{
		Timer.first = std::chrono::high_resolution_clock::now();

		Log::Write("Main", "Tick [" + std::to_string(NowTickNum) + "]");

		std::queue<std::vector<uint8_t>> Events;

		{
			std::unique_lock<std::mutex> EventQueueLock(EventQueueMutex);
			Events.swap(EventQueue);
		}

		// 追加服务器Tick事件
		// 第一字节表示服务器事件 第二字节表示是Tick事件 紧跟8字节为Tick序号
		Events.push({ 0ui8, 'T', 
			(uint8_t)(NowTickNum >>  0),
			(uint8_t)(NowTickNum >>  8), 
			(uint8_t)(NowTickNum >> 16), 
			(uint8_t)(NowTickNum >> 24), 
			(uint8_t)(NowTickNum >> 32), 
			(uint8_t)(NowTickNum >> 40), 
			(uint8_t)(NowTickNum >> 48),
			(uint8_t)(NowTickNum >> 56)
			});

		for (auto x : Players)
		{
			std::unique_lock<std::mutex> PlayerEventQueueLock(x->EventQueueMutex);
			x->EventQueue.push(Events);
		}

		NowTickNum++;
		Timer.second = std::chrono::high_resolution_clock::now();
		RealDifferenceTime = std::chrono::duration_cast<std::chrono::milliseconds>(Timer.second - Timer.first);
		std::this_thread::sleep_for(TheoryDifferenceTime - RealDifferenceTime);
	}

}

void Server::Stop()
{
	Stopping = true;
}

void Server::SocketFunction()
{
	int32_t BytesNum;
	InternetAddr ClientAddr;
	std::shared_ptr<Socket> ClientSocket;
	std::vector<uint8_t> Data;
	while (!Stopping)
	{
		MainSocket->Listen(0);
		ClientSocket = std::shared_ptr<Socket>(MainSocket->Accept(ClientAddr));

		if (ClientSocket)
		{
			if (SocketHelper::SendWithTimeout(ClientSocket, { (uint8_t)(NowPlayerID >> 0), (uint8_t)(NowPlayerID >> 8) }, BytesNum)
				&& SocketHelper::RecvWithTimeout(ClientSocket, Data, BytesNum))
			{
				std::shared_ptr<Player> NewPlayer = std::shared_ptr<Player>(new Player);
				NewPlayer->ID = NowPlayerID;
				NewPlayer->Description = SocketHelper::ArrayToString(Data);
				NewPlayer->OwnSocket = ClientSocket;
				NewPlayer->Stopping = false;
				NewPlayer->Thread = std::shared_ptr<std::thread>(new std::thread(&Server::PlayerFunction, this, NewPlayer));

				Players.push_back(NewPlayer);
			}
		}

		ClientSocket = nullptr;
	}
}

void Server::PlayerFunction(std::weak_ptr<class Player> Owner)
{
}
