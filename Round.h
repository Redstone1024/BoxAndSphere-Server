#pragma once

#include "Support/Utils.h"

#include "Event.h"
#include "RoundPass.h"

#include <memory>
#include <map>
#include <mutex>
#include <vector>
#include <queue>
#include <string>

class Round
{
private:
	bool Stopping = false;  // 回合正在被销毁
	bool Destroyed = false; // 回合已经过期

	RoundPass Pass; // 回合的验证值

	std::string LogChannel; // 当前回合使用的Log通道

public:
	Round(RoundPass pPass);

	~Round();

	void Run(); // 主函数

	void Stop(); // 外部线程请求中途结束

	void AddByteStream(std::shared_ptr<class ByteStream> Stream, unsigned int ID); // 异步添加一个新的连接

	RoundPass GetPass() const { return Pass; } // 获取回合的验证值

	bool IsDestroyed() const { return Destroyed; } // 回合过期了吗

private:
	uint32_t TickCount = 0; // 下一个Tick的编号
	uint8_t FPS = 30;       // Tick帧率
	
	void Tick();                 // 每Tick调用
	void SetFPS(uint8_t NewFPS); // 更改帧率

private:
	uint32_t NextID = 0;              // 下一个事件的编号
	std::vector<uint8_t> MessageTemp; // 消息缓冲
	std::vector<uint8_t> SendMessage; // 发送的所有消息

	void SendEvent(const Event& NewEvent); // 发送一个消息

	// 所有连接到的客户端的信息
	struct ByteStreamInfo
	{
		std::shared_ptr<class ByteStream> Stream; // 流
		size_t NextMessageIndex = 0;              // 下一个要处理的字节索引
		std::vector<uint8_t> Message;             // 接收到的消息
	};

	std::chrono::seconds TimeoutLimit;                  // 超时限制
	std::vector<unsigned int> ToRemoveByteStreams;      // 在处理结尾需要被删除的客户端
	std::map<unsigned int, ByteStreamInfo> ByteStreams; // 客户端池

	std::mutex ToAddByteStreamsMutex;                                                         // 待异步加入的客户端数组的互斥锁
	std::vector<std::pair<unsigned int, std::shared_ptr<class ByteStream>>> ToAddByteStreams; // 待异步加入的客户端

	void HandleNewByteStream();  // 处理待异步加入的客户端
	void HandleByteStreamRecv(); // 处理接收到消息

};
