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
 * ���õ��µ�����ʱ��
 * ��ȡ17���ַ�����ȡ����������ʱ���͡�F����ʾЭ�����
 * ��һ���ַ�Ϊ�������ͣ�����8���ֽ�Ϊ�غϱ�ţ���8��Ϊ����
 * ��һ���ַ�Ϊ��R��ʱ����ʾ��Ҫע��һ���غϣ�����RegisterRound
 * ��һ���ַ�Ϊ��L��ʱ����ʾ��Ҫ����һ���غϣ�����JoinRound
 *
 * F ��ʾЭ�����
 * K ��ʾ�����Ѿ�����
 * I ��ʾ���䲻����
 * P ��ʾ�������
 * T ��ʾ���ӳɹ�
 * 
 * ���غ�������ʱ����
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
