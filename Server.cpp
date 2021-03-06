#include "Server.h"

#include "Round.h"
#include "Support/Log.h"
#include "Network/NetworkByteStream/ByteStream.h"
#include "Network/NetworkByteStream/BytesHelper.h"
#include "Network/NetworkByteStream/ConnectListener.h"
#include "Network/NetworkByteStream/ConnectMaker/ConnectMakerTCP.h"

#include <chrono>
#include <string>

Server::Server(Arguments Arg)
	: ServerArguments(Arg)
	, TimeoutLimit(3)
{
	// 添加Log通道
	Log::AddChannel("Server", "Server", true);

	// 解析服务器设置
	Setting.IP = ServerArguments.GetValue("IP", "Default");
	Setting.Port = std::stoi(ServerArguments.GetValue("Port", "25565"));

	// 启动监听者
	AvailablePactsType AvailablePacts = 
	{ 
		std::shared_ptr<ConnectServerMaker>(new ConnectServerMakerTCP()),
	};

	Listener = std::shared_ptr<ConnectListener>(new ConnectListener(Setting.IP, Setting.Port, AvailablePacts));
}

Server::~Server()
{
	Stop();
}

void Server::Run()
{
	Log::Write("Server", "Server Open");
	Listener->Start();

	std::shared_ptr<ByteStream> NewConnection;
	while (!Stopping)
	{
		// 获取新的连接并加入处理池
		do 
		{
			NewConnection = Listener->TryGetConnection();
			if (NewConnection) AddNewConnection(NewConnection);
		} 
		while (NewConnection);

		// 处理连接
		HandleConnection();

		// 处理回合
		HandleRounds();

		// 休眠一会减少服务器CPU占用
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	Listener->Stop();
	CloseRounds();

	Log::Write("Server", "Server Close");
}

void Server::Stop()
{
	Stopping = true;
}

void Server::AddNewConnection(std::shared_ptr<ByteStream> NewConnection)
{
	ProcConnection NewProcConnection;
	NewProcConnection.Stream = NewConnection;
	ProcConnections[++NextConnectionID] = NewProcConnection; // 拿到一个未使用的连接ID
	Log::Write("Server", "New Customers Join [" +  std::to_string(NextConnectionID) + "]");
}

void Server::HandleConnection()
{
	// F 表示协议错误
	ToRemoveConnections.clear();
	for (auto Connection : ProcConnections)
	{
		// 获取新消息并追加到消息缓冲
		bool HasData = false;
		do
		{
			NewMessageBuffer.clear();
			Connection.second.Stream->Recv(NewMessageBuffer, 17 - Connection.second.Message.size());
			if (!NewMessageBuffer.empty())
			{
				Connection.second.Message.insert(Connection.second.Message.end(), NewMessageBuffer.begin(), NewMessageBuffer.end());
				HasData = true;
			}
		} while (!NewMessageBuffer.empty());

		if (HasData)
		{
			// 处理新的消息

			Connection.second.Message.insert(Connection.second.Message.end(), NewMessageBuffer.begin(), NewMessageBuffer.end());

			std::vector<uint8_t>& Message = Connection.second.Message;
			if (Message.size() == 17)
			{
				// 满足处理条件开始处理

				RoundPass NewRoundPass;
				NewRoundPass.RoundNumber = BYTESTOINT64(Message.data() + 1);
				NewRoundPass.Password = BYTESTOINT64(Message.data() + 9);

				if (Message[0] == 'R')
					RegisterRound(NewRoundPass, Connection.second.Stream, Connection.first);
				else if (Message[0] == 'L')
					JoinRound(NewRoundPass, Connection.second.Stream, Connection.first);
				else
				{
					Log::Write("Server", "Customers Unknown Command [" + std::to_string(Connection.first) + "]");
					Connection.second.Stream->Send({ 'F' });
				}

				// 处理完成删除该连接
				ToRemoveConnections.push_back(Connection.first);
			}
		}
		else
		{
			// 查看连接是否超时
			std::chrono::system_clock::time_point NewTime = std::chrono::system_clock::now();
			std::chrono::seconds RealDifferenceTime = std::chrono::duration_cast<std::chrono::seconds>(NewTime - Connection.second.Stream->GetLastActiveTime());
			if (RealDifferenceTime > TimeoutLimit)
			{
				// 超时则删除
				ToRemoveConnections.push_back(Connection.first);
				Connection.second.Stream->Send({ 'F' });
				Log::Write("Server", "Customers Connection Timeout [" + std::to_string(Connection.first) + "]");
			}
		}
	}

	// 真正删除连接的地方
	for (auto ID : ToRemoveConnections)
	{
		ProcConnections.erase(ID);
		Log::Write("Server", "Customers Connection Removed [" + std::to_string(ID) + "]");
	}
}

void Server::RegisterRound(RoundPass Pass, std::shared_ptr<ByteStream> Stream, unsigned int ConnectionID)
{
	// K 表示房间已经存在
	if (Rounds.find(Pass.RoundNumber) == Rounds.end())
	{
		Stream->Send({ 'T' });
		RoundInfo NewRound;
		NewRound.Self = std::shared_ptr<Round>(new Round(Pass));
		NewRound.Self->AddByteStream(Stream, ConnectionID);
		NewRound.Thread = std::shared_ptr<std::thread>(new std::thread(&Round::Run, NewRound.Self.get()));
		Rounds[Pass.RoundNumber] = NewRound;
		Log::Write("Server", "Round Construct [" + std::to_string(Pass.RoundNumber) + "]");
		Log::Write("Server", "Connection [" + std::to_string(ConnectionID) + "] Join To Round [" + std::to_string(Pass.RoundNumber) + "] Succeed");
	}
	else
	{
		Stream->Send({ 'K' });
		Log::Write("Server", "Connection [" + std::to_string(ConnectionID) + "] Join To Round [" + std::to_string(Pass.RoundNumber) + "] Fail (The Round Is Already Registered)");
	}
}

void Server::JoinRound(RoundPass Pass, std::shared_ptr<ByteStream> Stream, unsigned int ConnectionID)
{
	// I 表示房间不存在
	// P 表示密码错误
	// T 表示连接成功
	if (Rounds.find(Pass.RoundNumber) != Rounds.end())
	{
		if (Rounds.at(Pass.RoundNumber).Self->GetPass().Password == Pass.Password)
		{
			Stream->Send({ 'T' });
			Rounds.at(Pass.RoundNumber).Self->AddByteStream(Stream, ConnectionID);
			Log::Write("Server", "Connection [" + std::to_string(ConnectionID) + "] Join To Round [" + std::to_string(Pass.RoundNumber) + "] Succeed");
		}
		else
		{
			Stream->Send({ 'P' });
			Log::Write("Server", "Connection [" + std::to_string(ConnectionID) + "] Join To Round [" + std::to_string(Pass.RoundNumber) + "] Fail (Wrong Password)");
		}
	}
	else
	{
		Stream->Send({ 'I' });
		Log::Write("Server", "Connection [" + std::to_string(ConnectionID) + "] Join To Round [" + std::to_string(Pass.RoundNumber) + "] Fail (The Round Not Registered)");
	}
}

void Server::HandleRounds()
{
	// 将已经过期的回合删除
	ToRemoveRounds.clear();
	for (auto tRound : Rounds)
	{
		if (tRound.second.Self->IsDestroyed())
		{
			if (tRound.second.Thread->joinable())
				tRound.second.Thread->join();
			ToRemoveRounds.push_back(tRound.first);
		}
	}

	for (auto ToRemoveRound : ToRemoveRounds)
	{
		Rounds.erase(ToRemoveRound);
		Log::Write("Server", "Round Destroyed [" + std::to_string(ToRemoveRound) + "]");
	}
}

void Server::CloseRounds()
{
	for (auto tRound : Rounds)
	{
		tRound.second.Self->Stop();
		if (tRound.second.Thread->joinable())
			tRound.second.Thread->join();
	}
	Rounds.clear();
}
