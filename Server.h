#pragma once

#include "Support/Utils.h"

#include "Support/Arguments.h"
#include "ServerSetting.h"
#include "RoundPass.h"

#include <map>
#include <memory>
#include <chrono>
#include <vector>
#include <thread>

/**
 * 当拿到新的连接时：
 * 读取17个字符，读取超出数量或超时发送“F”表示协议错误，
 * 第一个字符为操作类型，接着8个字节为回合编号，再8个为密码
 * 第一个字符为“R”时，表示想要注册一个回合，调用RegisterRound
 * 第一个字符为“L”时，表示想要加入一个回合，调用JoinRound
 *
 * F 表示协议错误
 * K 表示房间已经存在
 * I 表示房间不存在
 * P 表示密码错误
 * T 表示连接成功
 * 
 * 当回合无连接时销毁
 * 
 */

class Server
{
private:
	bool Stopping = false;

	Arguments ServerArguments;
	ServerSetting Setting;

	std::shared_ptr<class ConnectListener> Listener;

public:
	Server(Arguments Arg);

	~Server();

	void Run();

	void Stop();

	bool IsStopping() const { return Stopping; }

private:
	struct ProcConnection
	{
		std::shared_ptr<class ByteStream> Stream;
		std::vector<uint8_t> Message;
	};

	unsigned int NextConnectionID = 0;
	std::map<unsigned int, ProcConnection> ProcConnections;

	std::chrono::seconds TimeoutLimit;
	std::vector<uint8_t> NewMessageBuffer;
	std::vector<unsigned int> ToRemoveConnections;

	void AddNewConnection(std::shared_ptr<class ByteStream> NewConnection);
	void HandleConnection();

private:
	struct RoundInfo
	{
		std::shared_ptr<class Round> Self;
		std::shared_ptr<std::thread> Thread;
	};

	std::map<uint64_t, RoundInfo> Rounds;
	std::vector<uint64_t> ToRemoveRounds;

	void RegisterRound(RoundPass Pass, std::shared_ptr<class ByteStream> Stream, unsigned int ConnectionID);
	void JoinRound(RoundPass Pass, std::shared_ptr<class ByteStream> Stream, unsigned int ConnectionID);
	void HandleRounds();
	void CloseRounds();

};
