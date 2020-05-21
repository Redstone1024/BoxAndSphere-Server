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
 * ������[ConnectListener]�������ڷ�������������һ���̼߳������������յ������
 * ������Ҫ��������[TCP/UDP��]������֮��Ӧ�Ĵ����ߡ�
 *
 * �����ֽ�������˼·��
 * ������                                �ͻ���
 * ������                                             ������
 * |  ��ʼ�������ȴ��ͻ��˵����� <------ ������������      |
 * |  ������յ��󣬲鿴�����ͱ�� <---- ���������ͱ��    |
 * |  ���������ͱ�ţ����ö�Ӧ������                       |
 * |������                                                 |
 * || ������                                               |
 * || ���ͽ��������Ϣ ----------------> �յ������ɹ���Ϣ  |
 * || ������ָ��                         ������ָ��        |
 * |  ����ָ��ѹ����еȴ����߳���ȡ
 * |  �ȴ�����ͻ�������
 */

class ConnectListener
{
	friend std::thread;
	
public:
	ConnectListener(const std::string& IP, unsigned short Port, const AvailablePactsType& AvailablePacts);
	~ConnectListener();

	// ���Ƽ����Ŀ�ʼ�ͽ���
	bool Start();
	void Stop();

	// �Ƿ��ڼ���
	bool IsListening() { return Listening; }

	// ��ȡ���µļ������
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
