#include "Log.h"

#include <iostream>
#include <chrono>
#include <iomanip>

#pragma warning(disable:4996)

const std::string Log::FileRxtension = ".log";

std::map<std::string, ChannelInfo> Log::mChannels;
std::mutex Log::mChannelMutex;
std::mutex Log::mWriteMutex;

ChannelInfo& Log::Channel(const std::string& ChannelName)
{
	const std::lock_guard<std::mutex> Lock(mChannelMutex);

	auto It = mChannels.find(ChannelName);
	if (It == mChannels.end())
		throw std::invalid_argument("Tried logging to non-existent channel " + ChannelName);
	
	return It->second;
}

void Log::AddChannel(const std::string& ChannelName, const std::string& BaseFilename, bool IsTimestamped)
{
	const std::lock_guard<std::mutex> Lock(mChannelMutex);

	if (mChannels.find(ChannelName) != mChannels.end()) return;

	if (BaseFilename.empty()) return;

	std::string Path = "Logs";
	CreateDirectory(Path);

	std::string Filename(BaseFilename + FileRxtension);
	mChannels.insert(std::map<std::string, ChannelInfo>::value_type(ChannelName, ChannelInfo(Filename, IsTimestamped, Path + "/" + Filename)));
	return;
}

void Log::Write(const std::string& ChannelName, const std::string& Str, bool ToScreen)
{
	const std::lock_guard<std::mutex> Lock(mWriteMutex);

	ChannelInfo& tChannel = Channel(ChannelName);
	std::ofstream& tOf = tChannel.Writer;
	if (!tOf) return;

	if (!tChannel.IsTimestamped)
	{
		tOf << Str << std::endl;
		if (ToScreen) std::cout << Str << std::endl;
	}
	else
	{
		std::chrono::system_clock::time_point n1 = std::chrono::system_clock::now();
		std::time_t t = std::chrono::system_clock::to_time_t(n1);
		tOf << "[" << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S") << " "
			<< std::setw(4) << std::setfill('0')
			<< std::chrono::time_point_cast<std::chrono::milliseconds>(n1).time_since_epoch().count() % 1000 << "] " 
			<< Str << std::endl;

		if (ToScreen) std::cout << "[" << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S") << " "
			<< std::setw(4) << std::setfill('0')
			<< std::chrono::time_point_cast<std::chrono::milliseconds>(n1).time_since_epoch().count() % 1000 << "] "
			<< Str << std::endl;
	}
}
