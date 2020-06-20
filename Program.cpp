#include "Support/Arguments.h"

#include "Server.h"

#include <string>
#include <thread>
#include <iostream>
#include <algorithm>

// 控制台命令监听
void CMDListener(Server* S)
{
	std::string Str;
	while (!S->IsStopping())
	{
		std::cin >> Str;
		std::transform(Str.begin(), Str.end(), Str.begin(), ::toupper);

		if (Str == "STOP")
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
