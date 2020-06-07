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
	bool Stopping = false;  // �غ����ڱ�����
	bool Destroyed = false; // �غ��Ѿ�����

	RoundPass Pass; // �غϵ���ֵ֤

	std::string LogChannel; // ��ǰ�غ�ʹ�õ�Logͨ��

public:
	Round(RoundPass pPass);

	~Round();

	void Run(); // ������

	void Stop(); // �ⲿ�߳�������;����

	void AddByteStream(std::shared_ptr<class ByteStream> Stream, unsigned int ID); // �첽���һ���µ�����

	RoundPass GetPass() const { return Pass; } // ��ȡ�غϵ���ֵ֤

	bool IsDestroyed() const { return Destroyed; } // �غϹ�������

private:
	uint32_t TickCount = 0; // ��һ��Tick�ı��
	uint8_t FPS = 30;       // Tick֡��
	
	void Tick();                 // ÿTick����
	void SetFPS(uint8_t NewFPS); // ����֡��

private:
	uint32_t NextID = 0;              // ��һ���¼��ı��
	std::vector<uint8_t> MessageTemp; // ��Ϣ����
	std::vector<uint8_t> SendMessage; // ���͵�������Ϣ

	void SendEvent(const Event& NewEvent); // ����һ����Ϣ

	// �������ӵ��Ŀͻ��˵���Ϣ
	struct ByteStreamInfo
	{
		std::shared_ptr<class ByteStream> Stream; // ��
		size_t NextMessageIndex = 0;              // ��һ��Ҫ������ֽ�����
		std::vector<uint8_t> Message;             // ���յ�����Ϣ
	};

	std::chrono::seconds TimeoutLimit;                  // ��ʱ����
	std::vector<unsigned int> ToRemoveByteStreams;      // �ڴ����β��Ҫ��ɾ���Ŀͻ���
	std::map<unsigned int, ByteStreamInfo> ByteStreams; // �ͻ��˳�

	std::mutex ToAddByteStreamsMutex;                                                         // ���첽����Ŀͻ�������Ļ�����
	std::vector<std::pair<unsigned int, std::shared_ptr<class ByteStream>>> ToAddByteStreams; // ���첽����Ŀͻ���

	void HandleNewByteStream();  // ������첽����Ŀͻ���
	void HandleByteStreamRecv(); // ������յ���Ϣ

};
