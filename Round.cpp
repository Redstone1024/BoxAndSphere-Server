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

	std::chrono::nanoseconds TheoryDifferenceTime(std::chrono::nanoseconds(std::chrono::seconds(1)) / FPS);
	std::chrono::nanoseconds RealDifferenceTime;

	std::chrono::system_clock::time_point LastTickTime = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point NowTime;

	while (!Stopping && !(ByteStreams.empty() && ToAddByteStreams.empty()))
	{
		HandleNewByteStream();
		HandleByteStreamRecv();

		NowTime = std::chrono::system_clock::now();
		RealDifferenceTime = std::chrono::duration_cast<std::chrono::milliseconds>(NowTime - LastTickTime);
		if (RealDifferenceTime >= TheoryDifferenceTime)
		{
			Tick();
			LastTickTime = NowTime - (RealDifferenceTime - TheoryDifferenceTime);
		}

		std::this_thread::sleep_for(std::chrono::nanoseconds(std::chrono::seconds(1)) / (FPS * 1000));
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

void Round::Tick()
{
	Event TickEvent;
	TickEvent.CMD = 1;
	TickEvent.Params = { INT32TOBYTES(TickCount) };
	SendEvent(TickEvent);

	TickCount++;
}

void Round::SetFPS(uint8_t NewFPS)
{
	if (NewFPS == 0)
		NewFPS = 1;

	if (NewFPS > 100)
		NewFPS = 100;

	FPS = NewFPS;
}

void Round::SendEvent(const Event & NewEvent)
{
	uint32_t NowID = NextID++;
	MessageTemp = { INT32TOBYTES(NowID), INT32TOBYTES(NewEvent.CMD), INT32TOBYTES(NewEvent.Params.size()) };
	MessageTemp.insert(MessageTemp.end(), NewEvent.Params.begin(), NewEvent.Params.end());

	uint8_t tCheck = 0;
	for (auto Byte : MessageTemp)
		tCheck ^= Byte;

	MessageTemp.push_back(tCheck);

	MessageStream.insert(MessageStream.end(), MessageTemp.begin(), MessageTemp.end());
	
	for (auto Connection : ByteStreams)
		Connection.second.Stream->Send(MessageTemp);

	MessageTemp.clear();
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

void Round::HandleByteStreamRecv()
{
	ToRemoveByteStreams.clear();
	for (auto Connection : ByteStreams)
	{
		bool HasData = false;
		do 
		{
			MessageTemp.clear();
			Connection.second.Stream->Recv(MessageTemp);
			if (!MessageTemp.empty())
			{
				Connection.second.Message.insert(Connection.second.Message.end(), MessageTemp.begin(), MessageTemp.end());
				HasData = true;
			}
		}
		while (!MessageTemp.empty());

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
					SendEvent(NewEvent);

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
