#pragma once

#include "../../Support/Utils.h"

#include <memory>

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
	virtual void Recv(std::vector<uint8_t>& Data) = 0;
	virtual void Update() { }

	ByteStream(const ByteStream& rhs) = delete;
	ByteStream(const ByteStream&& rhs) = delete;
	ByteStream& operator = (const ByteStream& rhs) = delete;
	ByteStream& operator = (const ByteStream&& rhs) = delete;
};
