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
	bool Stopping = false;
	bool Destroyed = false;

	RoundPass Pass;

	std::string LogChannel;

public:
	Round(RoundPass pPass);

	~Round();

	void Run();

	void Stop();

	void AddByteStream(std::shared_ptr<class ByteStream> Stream, unsigned int ID);

	RoundPass GetPass() const { return Pass; }

	bool IsDestroyed() const { return Destroyed; }

private:
	uint32_t TickCount = 0;
	uint8_t FPS = 30;
	
	void Tick();
	void SetFPS(uint8_t NewFPS);

private:
	uint32_t NextID = 0;
	std::vector<uint8_t> MessageTemp;
	std::vector<uint8_t> MessageStream;

	std::vector<uint8_t> EventMessage;

	void SendEvent(const Event& NewEvent);

private:
	struct ByteStreamInfo
	{
		std::shared_ptr<class ByteStream> Stream;
		size_t NextMessageIndex = 0;
		std::vector<uint8_t> Message;
	};

	std::chrono::seconds TimeoutLimit;
	std::vector<unsigned int> ToRemoveByteStreams;
	std::map<unsigned int, ByteStreamInfo> ByteStreams;

	std::mutex ToAddByteStreamsMutex;
	std::vector<std::pair<unsigned int, std::shared_ptr<class ByteStream>>> ToAddByteStreams;

	void HandleNewByteStream();
	void HandleByteStreamRecv();

};
