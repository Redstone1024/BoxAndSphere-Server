#pragma once

#include "../../Support/Utils.h"

#include <memory>

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
	virtual void Recv(std::vector<uint8_t>& Data) = 0;
	virtual void Update() { }

	ByteStream(const ByteStream& rhs) = delete;
	ByteStream(const ByteStream&& rhs) = delete;
	ByteStream& operator = (const ByteStream& rhs) = delete;
	ByteStream& operator = (const ByteStream&& rhs) = delete;
};
