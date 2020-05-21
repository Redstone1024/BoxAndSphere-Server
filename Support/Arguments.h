#pragma once

#include "Utils.h"

#include <map>
#include <string>

class Arguments
{
private:
	std::map<std::string, std::string> mArgs;

public:
	Arguments() = default;
	Arguments(int Argc, char* Argv[]);

	bool Contains(const std::string& Key) const { return mArgs.find(Key) != mArgs.end(); }
	const std::string& GetValue(const std::string& Key, const std::string& DefaultValue) const { return Contains(Key) ? mArgs.at(Key) : DefaultValue; }
	void ReplaceValue(const std::string& Key, const std::string& Value) { mArgs[Key] = Value; }
};
