#pragma once

#include "Utils.h"

#include <cmath>
#include <ctime>

class MersenneTwister
{
private:
	const static size_t mMTSize = 624;
	unsigned int mMT[mMTSize];
	int mIndex = 0;

	int mLast;
	int TotalCount = 0;

public:
	MersenneTwister() : MersenneTwister(static_cast<int>(std::time(nullptr))) {}

	MersenneTwister(int Seed);

	int Next();

	int Next(int Low, int High);

	int Next(int High) { return Next(0, High); }

	float NextFloat() { return std::abs(Next() / (float)0x7fffffff); }

	void Generate();
};
