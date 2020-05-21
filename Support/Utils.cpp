#include "Utils.h"

#include <direct.h>

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
		Success = _mkdir((Temp += x).c_str()) == 0 ? true : false;
		Temp += "/";
	}
	return Success;
}
