#pragma once

#include "../../Support/Utils.h"

#include <memory>
#include <chrono>

/**
 * 网络字节流[ByteStream]，作为网络会话的代码，只有基础的收发功能。
 */

class ByteStream
{
protected:
	ByteStream() { };

public:
	virtual ~ByteStream() { }

	virtual void Send(const std::vector<uint8_t>& Data) = 0;
	virtual void Recv(std::vector<uint8_t>& Data, uint32_t MaxBytesRead = UINT32_MAX) = 0;
	virtual void Update() { }

	virtual std::chrono::system_clock::time_point GetLastActiveTime() = 0;

	ByteStream(const ByteStream& rhs) = delete;
	ByteStream(const ByteStream&& rhs) = delete;
	ByteStream& operator = (const ByteStream& rhs) = delete;
	ByteStream& operator = (const ByteStream&& rhs) = delete;
};
