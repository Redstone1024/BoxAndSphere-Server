#pragma once

#include "../../Support/Utils.h"

#include <memory>
#include <chrono>

/**
 * �����ֽ���[ByteStream]����Ϊ����Ự�Ĵ��룬ֻ�л������շ����ܡ�
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
