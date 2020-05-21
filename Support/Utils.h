#pragma once

#ifdef linux
#define _GLIBCXX_USE_NANOSLEEP
#endif // linux

#include <vector>
#include <string>

std::vector<std::string> Split(const std::string& Str, const std::string& Pattern);

bool CreateDirectory(const std::string& Path);

inline std::string TrimL(const std::string& Str)
{
	size_t Start = Str.find_first_not_of(" \n\r\t\f\v");
	return (Start == std::string::npos) ? "" : Str.substr(Start);
}

inline std::string TrimR(const std::string& Str)
{
	size_t End = Str.find_last_not_of(" \n\r\t\f\v");
	return (End == std::string::npos) ? "" : Str.substr(0, End + 1);
}

inline std::string Trim(const std::string& Str)
{
	return TrimR(TrimL(Str));
}
