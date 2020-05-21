#pragma once

#include "../../Support/Utils.h"

class ByteStream;

/**
 * �����ߣ�
 * ��Ϊ����˴�����[ConnectServerMaker]���Ϳͻ��˴�����[ConnectClientMaker]
 * �໥��Ӧ�����ڴ�����ͬ��Э��ջ��������������ֽ�����
 * �ͻ��˴����߹��������е�PactĬ��ֵӦ���������������ͬ
 * �����������ߵ�IDӦ���ڹ��������и�����ΨһID
 *
 * ע�⣺��PactΪ0��ʱ����Ҫ�����Լ�����ID��������ֱ�Ӵ�0��ȥ��
 */

class ConnectClientMaker
{
public:
	virtual std::shared_ptr<ByteStream> Construct(const std::string& IP, unsigned short Port, uint8_t Pact = 0 /* �˴������������ID��ͬ */) = 0;
};

class ConnectServerMaker
{
public:
	virtual uint8_t GetPactID() { return 0 /* �˴���ͻ��˲���ID��ͬ */ ; }
	virtual std::shared_ptr<ByteStream> Construct(std::shared_ptr<class Socket> Sock) = 0;
};
