#pragma once

#include "../Support/Utils.h"

#include <string>
#include <memory>
#include <vector>

class Socket;

struct LockstepPassInfo
{
	std::string Pass;
	std::string Send;

	LockstepPassInfo() = default;

	LockstepPassInfo(const std::string& Str)
		: Pass("C" + Str)
		, Send("S" + Str)
	{ }

	LockstepPassInfo(const std::string& P, const std::string& S)
		: Pass(P)
		, Send(S)
	{ }
};

class SocketHelper
{
public:
	const static int32_t BufferSize = 512;

	static std::vector<uint8_t> StringToArray(const std::string& A);

	static std::string ArrayToString(const std::vector<uint8_t>& A);

	static bool SendWithTimeout(std::weak_ptr<Socket> Sock, const std::vector<uint8_t>& Data, int32_t& BytesSent, int WaitTimeMilli = 120);

	static bool RecvWithTimeout(std::weak_ptr<Socket> Sock, std::vector<uint8_t>& Data, int32_t& BytesRead, int WaitTimeMilli = 120);

	static bool ClientRequest(std::weak_ptr<Socket> Sock, const LockstepPassInfo& Info, int WaitTimeMilli = 120);
};
