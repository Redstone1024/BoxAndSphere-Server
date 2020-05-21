#include "MersenneTwister.h"

MersenneTwister::MersenneTwister(int Seed)
{
	mMT[0] = (unsigned int)Seed;
	for (int i = 1; i < mMTSize; i++)
		mMT[i] = 1812433253u * (mMT[i - 1] ^ (mMT[i - 1] >> 30)) + i;
}

int MersenneTwister::Next()
{
	if (mIndex == 0) Generate();

	unsigned int y = mMT[mIndex];
	y ^= y >> 11;
	y ^= (y << 7) & 2636928640;
	y ^= (y << 15) & 4022730752;
	y ^= y >> 18;

	mIndex = (mIndex + 1) % mMTSize;
	TotalCount++;
	mLast = (int)(y % 2147483647);
	return mLast;
}

int MersenneTwister::Next(int Low, int High)
{
	if (High < Low)
		throw "Maximum value is less than the minimum value.";

	int Diff = High - Low;
	if (Diff <= 1)
		return Low;

	return Low + Next() % Diff;
}

void MersenneTwister::Generate()
{
	for (int i = 0; i < mMTSize; i++)
	{
		int y = (mMT[i] & 0x80000000) | (mMT[(i + 1) % 624] & 0x7fffffff);
		mMT[i] = mMT[(i + 397u) % 624u] ^ (y >> 1);
		if ((y & 1) == 1)
			mMT[i] = mMT[i] ^ 2567483615;
	}
}
