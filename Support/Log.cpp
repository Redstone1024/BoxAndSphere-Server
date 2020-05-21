#include "Log.h"

#include <cstdio>
#include <ctime>

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

	char* pFilename = new char[FilenameLength];
	sprintf_s(pFilename, FilenameLength, "%s%s", BaseFilename.c_str(), FileRxtension.c_str());

	std::string Filename(pFilename);
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
		std::time_t tNow = std::time(0);
		char* tTimeStr = new char[TimeStringLength];
		ctime_s(tTimeStr, TimeStringLength, &tNow);
		tTimeStr[strlen(tTimeStr) - 1] = '\0';
		tOf << "[" << tTimeStr << "]" << Str << std::endl;
		if (ToScreen) std::cout << "[" << tTimeStr << "]" << Str << std::endl;
	}
}
