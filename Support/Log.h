#pragma once

#include "Utils.h"

#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>

struct ChannelInfo
{
	std::string Filename;
	bool IsTimestamped;
	std::ofstream Writer;

	ChannelInfo() = default;
	ChannelInfo(const std::string& pFilename, bool pIsTimestamped, const std::string& Path)
		: Filename(pFilename)
		, IsTimestamped(pIsTimestamped)
		, Writer()
	{ 
		Writer.open(Path);
		if (!Writer)
		{
			throw ("File failed to open " + Path).c_str();
		}
	}
};

class Log
{
private:
	const static size_t FilenameLength = 255;
	const static size_t TimeStringLength = 64;
	const static std::string FileRxtension;

	static std::map<std::string, ChannelInfo> mChannels;
	static std::mutex mChannelMutex;
	static std::mutex mWriteMutex;

public:
	static ChannelInfo& Channel(const std::string& ChannelName);
	static void AddChannel(const std::string& ChannelName, const std::string& BaseFilename, bool IsTimestamped = false);
	static void Write(const std::string& ChannelName, const std::string& Str, bool ToScreen = true);
};
