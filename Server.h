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
	bool Stopping = false; // ����Ƿ�ʼֹͣ

	// ����������
	Arguments ServerArguments; // ԭCMD����
	ServerSetting Setting;     // ������Ĳ���

	std::shared_ptr<class ConnectListener> Listener; // ������

public:
	Server(Arguments Arg);

	~Server();

	void Run(); // ������

	void Stop(); // �ⲿ�߳�������;����

	bool IsStopping() const { return Stopping; } // ����ֹͣ��

private:
	// ��ʾһ�������������
	struct ProcConnection
	{
		std::shared_ptr<class ByteStream> Stream; // ������
		std::vector<uint8_t> Message;             // �Ѿ����ܵ�����Ϣ
	};

	unsigned int NextConnectionID = 0;                      // ��һ��������ӵ�еı��
	std::map<unsigned int, ProcConnection> ProcConnections; // �����߳�

	std::chrono::seconds TimeoutLimit;             // ��ʱʱ��
	std::vector<uint8_t> NewMessageBuffer;         // ��Ϣ�ݴ滺��
	std::vector<unsigned int> ToRemoveConnections; // �ڴ����β��Ҫ��ɾ��������

	void AddNewConnection(std::shared_ptr<class ByteStream> NewConnection); // ���һ���µ�����
	void HandleConnection();                                                // ������������

private:
	// ��ʾһ���غ���Ϣ
	struct RoundInfo
	{
		std::shared_ptr<class Round> Self;   // �غ϶���
		std::shared_ptr<std::thread> Thread; // �غ��߳�
	};

	std::map<uint64_t, RoundInfo> Rounds; // �غ���Ϣ��
	std::vector<uint64_t> ToRemoveRounds; // �ڴ����β��Ҫ��ɾ���Ļغ�

	void RegisterRound(RoundPass Pass, std::shared_ptr<class ByteStream> Stream, unsigned int ConnectionID); // ע��һ���µĻغ�
	void JoinRound(RoundPass Pass, std::shared_ptr<class ByteStream> Stream, unsigned int ConnectionID);     // ����һ���Ѿ����ڵĻغ�
	void HandleRounds();                                                                                     // ����غ�
	void CloseRounds();                                                                                      // �ر����лغ�

};
