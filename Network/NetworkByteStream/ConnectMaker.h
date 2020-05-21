#pragma once

#include "../../Support/Utils.h"

class ByteStream;

/**
 * 创建者：
 * 分为服务端创建者[ConnectServerMaker]，和客户端创建者[ConnectClientMaker]
 * 相互对应，用于创建不同的协议栈，最后抽象成网络字节流。
 * 客户端创建者构建函数中的Pact默认值应与服务器创建者相同
 * 服务器构建者的ID应该在构建函数中给予与唯一ID
 *
 * 注意：当Pact为0是时，需要子类自己处理ID，并不是直接传0过去。
 */

class ConnectClientMaker
{
public:
	virtual std::shared_ptr<ByteStream> Construct(const std::string& IP, unsigned short Port, uint8_t Pact = 0 /* 此处与服务器部分ID相同 */) = 0;
};

class ConnectServerMaker
{
public:
	virtual uint8_t GetPactID() { return 0 /* 此处与客户端部分ID相同 */ ; }
	virtual std::shared_ptr<ByteStream> Construct(std::shared_ptr<class Socket> Sock) = 0;
};
