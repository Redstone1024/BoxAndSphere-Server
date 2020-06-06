#pragma once

#include "Support/Utils.h"

#include <vector>

struct Event
{
	uint32_t ID;
	uint32_t CMD;
	std::vector<uint8_t> Params;
	uint8_t Check;
};
