#include "Support/Arguments.h"

#include "Server.h"

#include <thread>
#include <iostream>

// 控制台命令监听
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
