#include "Support/Log.h"
#include "Support/Arguments.h"
#include "Support/MersenneTwister.h"

#include "Server.h"

#include <thread>

#include <iostream>
#include "Network/NetworkByteStream/ConnectMaker/ConnectMakerTCP.h"
#include <string>
#include "Network/NetworkByteStream/ByteStream.h"
#include "Network/NetworkByteStream/ConnectListener.h"
#include <memory>
#include "Network/NetworkByteStream/BytesHelper.h"
#include <memory.h>

bool Stopping;
void Print(std::shared_ptr<ByteStream> Stream)
{
	std::vector<uint8_t> Data;
	while (!Stopping)
	{
		Stream->Recv(Data);
		if (Data.size() > 0) std::cout << std::string((char*)Data.data()) << std::endl;
	}
}

void PrintInt32(std::shared_ptr<ByteStream> Stream)
{
	std::vector<uint8_t> Data;
	while (!Stopping)
	{
		Stream->Recv(Data);
		if (Data.size() == 4) std::cout << BYTESTOINT32(Data.data()) << std::endl;
	}
}

void CMDListener(Server* S)
{
	std::string Str;
	while (!S->IsStopping())
	{
		std::cin >> Str;
		if (Str == "Stop")
			S->Stop();
		else std::cout << "Unknown Command" << std::endl;
	}
}

int main(int argc, char *argv[])
{
	/*
	int x;
	std::string Str;
	std::vector<uint8_t> Data;
	//std::cin >> x;
	x = 1;

	std::shared_ptr<std::thread> Thread;

	Stopping = false;

	if (x == 1)
	{
		std::shared_ptr<ByteStream> Stream;
		ConnectListener Listener("Default", 25565, { std::shared_ptr<ConnectServerMaker>(new ConnectServerMakerTCP()) });
		Listener.Start();
		while (true)
		{
			Stopping = false;
			Stream = Listener.TryGetConnection();

			if (Stream)
				Thread = std::shared_ptr<std::thread>(new std::thread(&Print, Stream));

			while (Stream)
			{
				std::cin >> Str;
				Data.resize(Str.length() + 1);
				memcpy(Data.data(), Str.data(), Str.length());
				Data[Str.length()] = '\0';
				Stream->Send(Data);
			}

			Stopping = true;
			if (Thread && Thread->joinable())
				Thread->join();
			Thread = nullptr;
		}
		Listener.Stop();
	}

	if (x == 2)
	{
		ConnectClientMakerTCP ClientMaker;
		std::shared_ptr<ByteStream> Stream = ClientMaker.Construct("127.0.0.1", 25565);
		if (Stream == nullptr)
		{
			std::cout << "Connect Fail" << std::endl;
			return 0;
		}

		Thread = std::shared_ptr<std::thread>(new std::thread(&Print, Stream));

		while (true)
		{
			std::cin >> Str;
			if (Str == "exit") break;
			Data.resize(Str.length() + 1);
			memcpy(Data.data(), Str.data(), Str.length());
			Data[Str.length()] = '\0';
			Stream->Send(Data);
		}

		Stopping = true;
		if (Thread && Thread->joinable())
			Thread->join();
		Thread = nullptr;
	}

	if (x == 3)
	{
		ConnectClientMakerTCP ClientMaker;
		std::shared_ptr<ByteStream> Stream = ClientMaker.Construct("127.0.0.1", 25565);
		if (Stream == nullptr)
		{
			std::cout << "Connect Fail" << std::endl;
			return 0;
		}

		Thread = std::shared_ptr<std::thread>(new std::thread(&PrintInt32, Stream));

		while (true)
		{
			std::cin >> x;
			Data = { INT32TOBYTES(x) };
			Stream->Send(Data);
		}

		Stopping = true;
		if (Thread && Thread->joinable())
			Thread->join();
		Thread = nullptr;
	}
	*/
	
	Server S(Arguments(argc, argv));
	std::thread Listener(&CMDListener, &S);
	S.Run();
	if (Listener.joinable())
		Listener.join();
	return 0;
}
