#include "Utils.h"

#ifdef _WIN32

#include <direct.h>

#endif // _WIN32

#ifdef __linux__

#include <sys/stat.h>
#include <sys/types.h>

#endif // __linux__


std::vector<std::string> Split(const std::string &str, const std::string &pattern)
{
	std::vector<std::string> res;
	if (str == "")
		return res;

	std::string strs = str + pattern;
	std::size_t pos = strs.find(pattern);

	while (pos != strs.npos)
	{
		std::string temp = strs.substr(0, pos);
		res.push_back(temp);
		strs = strs.substr(pos + 1, strs.size());
		pos = strs.find(pattern);
	}

	return res;
}

bool CreateDirectory(const std::string& Path)
{
	std::vector<std::string> Subdirectory = Split(Path, "/");
	bool Success = true;
	std::string Temp;
	for (auto& x : Subdirectory)
	{
		Success = false;

#ifdef _WIN32
		Success = _mkdir((Temp += x).c_str()) == 0 ? true : false;
#endif // _WIN32

#ifdef __linux__
		Success = mkdir((Temp += x).c_str(), S_IRWXU) == 0 ? true : false;
#endif // __linux__

		Temp += "/";
	}
	return Success;
}
