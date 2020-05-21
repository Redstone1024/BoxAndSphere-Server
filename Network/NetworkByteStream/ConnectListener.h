#pragma once

#include "../../Support/Utils.h"

#include <memory>
#include <queue>
#include <mutex>
#include <thread>
#include <map>
#include <vector>

class ByteStream;
class ConnectServerMaker;

typedef std::vector<std::shared_ptr<ConnectServerMaker>> AvailablePactsType;
typedef std::map<uint8_t, std::shared_ptr<ConnectServerMaker>> AvailablePactsMap;

/**
 * 监听者[ConnectListener]，存在于服务器，单独开一个线程监听连接请求，收到请求后
 * 根据需要的流类型[TCP/UDP等]调用与之对应的创建者。
 *
 * 网络字节流构建思路：
 * 服务器                                客户端
 * 监听者                                             创建者
 * |  开始监听，等待客户端的请求 <------ 发送连接请求      |
 * |  请求接收到后，查看流类型编号 <---- 发送流类型编号    |
 * |  根据流类型编号，调用对应创建者                       |
 * |创建者                                                 |
 * || 建立流                                               |
 * || 发送建立完成信息 ----------------> 收到建立成功信息  |
 * || 返回流指针                         返回流指针        |
 * |  将流指针压入队列等待主线程来取
 * |  等待更多客户端请求
 */

class ConnectListener
{
	friend std::thread;
	
public:
	ConnectListener(const std::string& IP, unsigned short Port, const AvailablePactsType& AvailablePacts);
	~ConnectListener();

	// 控制监听的开始和结束
	bool Start();
	void Stop();

	// 是否在监听
	bool IsListening() { return Listening; }

	// 获取最新的监听结果
	std::shared_ptr<ByteStream> TryGetConnection();

private:
	void ListenFunction();

private:
	bool Stopping;

	std::string IP;
	unsigned short Port;
	AvailablePactsMap AvailablePacts;

	bool Listening;
	std::shared_ptr<class Socket> ListenSocket;
	std::shared_ptr<std::thread> ListenThread;

	std::mutex ConnectionQueueMutex;
	std::queue<std::shared_ptr<ByteStream>> ConnectionQueue;
};
