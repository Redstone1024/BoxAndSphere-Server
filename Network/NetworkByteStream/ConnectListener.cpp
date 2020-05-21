#include "ConnectListener.h"

#include "Socket.h"

#include "ConnectMaker.h"
#include "../../Support/Log.h"

ConnectListener::ConnectListener(const std::string & pIP, unsigned short pPort, const AvailablePactsType & pAvailablePacts)
	: Stopping(true)
	, Listening(false)
	, IP(pIP)
	, Port(pPort)
{
	Log::AddChannel("Network", "Network", true);
	Log::Write("Network", "ConnectListener Build Complete");

	for (auto& x : pAvailablePacts)
		AvailablePacts[x->GetPactID()] = x;
}

ConnectListener::~ConnectListener()
{
	Stop();
}

bool ConnectListener::Start()
{
	if (ListenThread != nullptr && Listening) return true;
	if (ListenThread != nullptr) Stop();
	Stopping = false;

	ListenSocket = std::shared_ptr<Socket>(new Socket());
	if (!ListenSocket->Bind({ IP, Port })) return false;
	Log::Write("Network", "ConnectListener Bind Success [" + IP + ":" + std::to_string(Port) + "]");
	ListenThread = std::shared_ptr<std::thread>(new std::thread(&ConnectListener::ListenFunction, this));
	return true;
}

void ConnectListener::Stop()
{
	Stopping = true;
	if (ListenSocket) ListenSocket->Close();
	if (ListenThread && ListenThread->joinable())
		ListenThread->join();
	ListenThread = nullptr;
}

std::shared_ptr<ByteStream> ConnectListener::TryGetConnection()
{
	const std::unique_lock<std::mutex> Lock(ConnectionQueueMutex);
	if (!ConnectionQueue.empty())
	{
		std::shared_ptr<ByteStream> Result = ConnectionQueue.front();
		ConnectionQueue.pop();
		return Result;
	}
	return std::shared_ptr<ByteStream>(nullptr);
}

void ConnectListener::ListenFunction()
{
	Listening = true;
	Log::Write("Network", "ConnectListener Start");

	InternetAddr ClientAddr;
	std::shared_ptr<Socket> ClientSock;
	std::shared_ptr<ByteStream> ClientByteStream;

	uint8_t TypeNumber;
	int32_t BytesRead;

	while (!Stopping)
	{
		if (!ListenSocket->Listen(0))
		{
			Log::Write("Network", "ConnectListener Listen Error");
			break;
		}

		ClientSock = std::shared_ptr<Socket>(ListenSocket->Accept(ClientAddr));
		Log::Write("Network", "ConnectListener Accept [" + ClientAddr.IP + ":" + std::to_string(ClientAddr.Port) + "]");

		if (ClientSock != nullptr)
		{
			if (!ClientSock->Wait(SocketWaitConditions::WaitForRead, 1000))
			{
				Log::Write("Network", "ConnectListener Recv Timeout [" + ClientAddr.IP + ":" + std::to_string(ClientAddr.Port) + "]");
				ClientSock = nullptr;
				continue;
			}

			if (!ClientSock->Recv(&TypeNumber, 1, BytesRead) || BytesRead != 1)
			{
				Log::Write("Network", "ConnectListener Recv Error [" + ClientAddr.IP + ":" + std::to_string(ClientAddr.Port) + "]");
				ClientSock = nullptr;
				continue;
			}

			if (AvailablePacts.find(TypeNumber) == AvailablePacts.end())
			{
				Log::Write("Network", "ConnectListener Unknown Pact [" + std::to_string(TypeNumber) + "] [" + ClientAddr.IP + ":" + std::to_string(ClientAddr.Port) + "]");
				ClientSock = nullptr;
				continue;
			}

			ClientByteStream = AvailablePacts[TypeNumber]->Construct(ClientSock);
			if (ClientByteStream)
			{
				Log::Write("Network", "Byte Stream Construct Success [" + std::to_string(TypeNumber) + "] [" + ClientAddr.IP + ":" + std::to_string(ClientAddr.Port) + "]");
				const std::unique_lock<std::mutex> Lock(ConnectionQueueMutex);
				ConnectionQueue.push(ClientByteStream);
			}
			else Log::Write("Network", "Byte Stream Construct Fail [" + std::to_string(TypeNumber) + "] [" + ClientAddr.IP + ":" + std::to_string(ClientAddr.Port) + "]");

		}

		ClientByteStream = nullptr;
		ClientSock = nullptr;
	}

	Log::Write("Network", "ConnectListener End");
	Listening = false;
}
