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
	Server S(Arguments(argc, argv));
	std::thread Listener(&CMDListener, &S);
	S.Run();
	if (Listener.joinable())
		Listener.join();
	return 0;
}
