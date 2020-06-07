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
	bool Stopping = false; // 标记是否开始停止

	// 服务器设置
	Arguments ServerArguments; // 原CMD参数
	ServerSetting Setting;     // 解析后的参数

	std::shared_ptr<class ConnectListener> Listener; // 监听器

public:
	Server(Arguments Arg);

	~Server();

	void Run(); // 主函数

	void Stop(); // 外部线程请求中途结束

	bool IsStopping() const { return Stopping; } // 正在停止吗

private:
	// 表示一个待处理的连接
	struct ProcConnection
	{
		std::shared_ptr<class ByteStream> Stream; // 流对象
		std::vector<uint8_t> Message;             // 已经接受到的消息
	};

	unsigned int NextConnectionID = 0;                      // 下一个连接者拥有的编号
	std::map<unsigned int, ProcConnection> ProcConnections; // 连接者池

	std::chrono::seconds TimeoutLimit;             // 超时时限
	std::vector<uint8_t> NewMessageBuffer;         // 消息暂存缓冲
	std::vector<unsigned int> ToRemoveConnections; // 在处理结尾需要被删除的连接

	void AddNewConnection(std::shared_ptr<class ByteStream> NewConnection); // 添加一个新的连接
	void HandleConnection();                                                // 处理所有连接

private:
	// 表示一个回合信息
	struct RoundInfo
	{
		std::shared_ptr<class Round> Self;   // 回合对象
		std::shared_ptr<std::thread> Thread; // 回合线程
	};

	std::map<uint64_t, RoundInfo> Rounds; // 回合信息池
	std::vector<uint64_t> ToRemoveRounds; // 在处理结尾需要被删除的回合

	void RegisterRound(RoundPass Pass, std::shared_ptr<class ByteStream> Stream, unsigned int ConnectionID); // 注册一个新的回合
	void JoinRound(RoundPass Pass, std::shared_ptr<class ByteStream> Stream, unsigned int ConnectionID);     // 加入一个已经存在的回合
	void HandleRounds();                                                                                     // 处理回合
	void CloseRounds();                                                                                      // 关闭所有回合

};
