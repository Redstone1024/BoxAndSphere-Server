#include "Round.h"

#include "Support/Utils.h"

#include "Event.h"
#include "Support/Log.h"
#include "Network/NetworkByteStream/ByteStream.h"
#include "Network/NetworkByteStream/BytesHelper.h"

#include <thread>

Round::Round(RoundPass pPass)
	: Pass(pPass)
	, LogChannel("Round-" + std::to_string(pPass.RoundNumber))
	, TimeoutLimit(5)
{ 
	Log::AddChannel(LogChannel, LogChannel, true);
	Log::Write(LogChannel, "Round Construct");
}

Round::~Round()
{
	Log::Write(LogChannel, "Round Destroyed");
}

void Round::Run()
{
	Log::Write(LogChannel, "Round Begin");

	std::pair<std::chrono::system_clock::time_point, std::chrono::system_clock::time_point> Timer;
	std::chrono::nanoseconds TheoryDifferenceTime(std::chrono::nanoseconds(std::chrono::seconds(1)) / FPS);
	std::chrono::nanoseconds RealDifferenceTime;

	while (!Stopping && !(ByteStreams.empty() && ToAddByteStreams.empty()))
	{
		Timer.first = std::chrono::system_clock::now();

		HandleNewByteStream();
		HandleByteStreamRecv();

		Event TickEvent;
		TickEvent.ID = NextID++;
		TickEvent.CMD = 1;
		TickEvent.Params = { INT32TOBYTES(TickCount) };
		AddEvent(TickEvent);

		TickCount++;

		HandleByteStreamSend();

		Timer.second = std::chrono::system_clock::now();
		RealDifferenceTime = std::chrono::duration_cast<std::chrono::milliseconds>(Timer.second - Timer.first);
		std::this_thread::sleep_for(TheoryDifferenceTime - RealDifferenceTime);		
	}

	Log::Write(LogChannel, "Round End");
	Destroyed = true;
}

void Round::Stop()
{
	Stopping = true;
}

void Round::AddByteStream(std::shared_ptr<class ByteStream> Stream, unsigned int ID)
{
	const std::unique_lock<std::mutex> Lock(ToAddByteStreamsMutex);

	ToAddByteStreams.push_back(std::pair<unsigned int, std::shared_ptr<class ByteStream>>(ID, Stream));
}

void Round::SetFPS(uint8_t NewFPS)
{
	if (NewFPS == 0)
		NewFPS = 1;

	if (NewFPS > 100)
		NewFPS = 100;

	FPS = NewFPS;
}

void Round::AddEvent(const Event & NewEvent)
{
	EventMessage = { INT32TOBYTES(NewEvent.ID), INT32TOBYTES(NewEvent.CMD), INT32TOBYTES(NewEvent.Params.size()) };
	EventMessage.insert(EventMessage.end(), NewEvent.Params.begin(), NewEvent.Params.end());

	uint8_t tCheck = 0;
	for (auto Byte : EventMessage)
		tCheck ^= Byte;

	EventMessage.push_back(tCheck);

	MessageQueue.push(EventMessage);

	EventMessage.clear();
}

void Round::HandleNewByteStream()
{
	const std::unique_lock<std::mutex> Lock(ToAddByteStreamsMutex);

	for (auto ToAddByteStream : ToAddByteStreams)
	{
		ByteStreamInfo NewByteStreamInfo;
		NewByteStreamInfo.Stream = ToAddByteStream.second;
		ByteStreams[ToAddByteStream.first] = NewByteStreamInfo;
		Log::Write(LogChannel, "Customers Connection Add [" + std::to_string(ToAddByteStream.first) + "]");
	}

	ToAddByteStreams.clear();
}

void Round::HandleByteStreamSend()
{
	if (!MessageQueue.empty())
	{
		for (auto Connection : ByteStreams)
		{
			std::queue<std::vector<uint8_t>> tMessageQueue = MessageQueue;
			while (!tMessageQueue.empty())
			{
				Connection.second.Stream->Send(tMessageQueue.front());
				tMessageQueue.pop();
			}
		}

		while (!MessageQueue.empty())
		{
			MessageStream.insert(MessageStream.end(), MessageQueue.front().begin(), MessageQueue.front().end());
			MessageQueue.pop();
		}
	}
}

void Round::HandleByteStreamRecv()
{
	ToRemoveByteStreams.clear();
	for (auto Connection : ByteStreams)
	{
		bool HasData = false;
		do 
		{
			NewMessageBuffer.clear();
			Connection.second.Stream->Recv(NewMessageBuffer);
			if (!NewMessageBuffer.empty())
			{
				Connection.second.Message.insert(Connection.second.Message.end(), NewMessageBuffer.begin(), NewMessageBuffer.end());
				HasData = true;
			}
		}
		while (!NewMessageBuffer.empty());

		if (HasData)
		{
			Event NewEvent;
			uint8_t tCheck;
			size_t NextMessageIndex = Connection.second.NextMessageIndex;
			std::vector<uint8_t>& Message = Connection.second.Message;

			while (Message.size() - NextMessageIndex >= 13)
			{

				size_t ParamsLen = BYTESTOINT32(Message.data() + NextMessageIndex + 8);
				if (ParamsLen > (Message.size() - NextMessageIndex - 1))
					break;

				NewEvent.ID = BYTESTOINT32(Message.data() + NextMessageIndex + 0);

				if (NewEvent.ID != 0)
				{
					ToRemoveByteStreams.push_back(Connection.first);
					Log::Write(LogChannel, "Customers Event ID Error [" + std::to_string(Connection.first) + "]");
					break;
				}

				NewEvent.ID = NextID++;

				NewEvent.CMD = BYTESTOINT32(Message.data() + NextMessageIndex + 4);
				NewEvent.Params.assign(Message.begin() + NextMessageIndex + 12, Message.begin() + NextMessageIndex + 12 + ParamsLen);
				NewEvent.Check = Message[NextMessageIndex + 12 + ParamsLen];

				tCheck = 0;
				for (size_t i = NextMessageIndex; i < NextMessageIndex + 12 + ParamsLen; i++)
					tCheck ^= Message[i];

				if (NewEvent.Check != tCheck)
				{
					ToRemoveByteStreams.push_back(Connection.first);
					Log::Write(LogChannel, "Customers Event Check Error [" + std::to_string(Connection.first) + "]");
					break;
				}

				if (NewEvent.CMD >= 16)
					AddEvent(NewEvent);

				NextMessageIndex += 12 + ParamsLen + 1;
			}
		}
		else
		{
			std::chrono::system_clock::time_point NewTime = std::chrono::system_clock::now();
			std::chrono::seconds RealDifferenceTime = std::chrono::duration_cast<std::chrono::seconds>(NewTime - Connection.second.Stream->GetLastActiveTime());
			if (RealDifferenceTime > TimeoutLimit)
			{
				ToRemoveByteStreams.push_back(Connection.first);
				Log::Write(LogChannel, "Customers Connection Timeout [" + std::to_string(Connection.first) + "]");
			}
		}
	}

	for (auto ID : ToRemoveByteStreams)
	{
		ByteStreams.erase(ID);
		Log::Write(LogChannel, "Customers Connection Removed [" + std::to_string(ID) + "]");
	}
}

/*
	std::pair<std::chrono::system_clock::time_point, std::chrono::system_clock::time_point> Timer;
	std::chrono::nanoseconds TheoryDifferenceTime(std::chrono::nanoseconds(std::chrono::seconds(1)) / Setting.FPS);
	std::chrono::nanoseconds RealDifferenceTime;

		Timer.first = std::chrono::system_clock::now();

		Timer.second = std::chrono::system_clock::now();
		RealDifferenceTime = std::chrono::duration_cast<std::chrono::milliseconds>(Timer.second - Timer.first);
		std::this_thread::sleep_for(TheoryDifferenceTime - RealDifferenceTime);

*/
