#include "Arguments.h"

#include <regex>

Arguments::Arguments(int Argc, char* Argv[])
{
	std::regex tRegex("([^=]+)=(.*)");
	for (int i = 0; i < Argc; i++)
	{
		std::cmatch tResult;
		if (std::regex_match(Argv[i], tResult, tRegex) && tResult.size() >= 3)
			mArgs[tResult[1]] = tResult[2];
	}

}
