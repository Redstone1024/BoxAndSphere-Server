#pragma once

#include "../../Support/Utils.h"

#define INT16TOBYTES(x) (uint8_t)((x) >>  0),\
						(uint8_t)((x) >>  8)

#define INT32TOBYTES(x) (uint8_t)((x) >>  0),\
						(uint8_t)((x) >>  8),\
						(uint8_t)((x) >> 16),\
						(uint8_t)((x) >> 24)


#define INT64TOBYTES(x) (uint8_t)((x) >>  0),\
						(uint8_t)((x) >>  8),\
						(uint8_t)((x) >> 16),\
						(uint8_t)((x) >> 24),\
						(uint8_t)((x) >> 32),\
						(uint8_t)((x) >> 40),\
						(uint8_t)((x) >> 48),\
						(uint8_t)((x) >> 56)

#define BYTESTOINT16(x) ((uint16_t)(*((x) + 0)) <<  0) + \
						((uint16_t)(*((x) + 1)) <<  8)

#define BYTESTOINT32(x) ((uint32_t)(*((x) + 0)) <<  0) + \
						((uint32_t)(*((x) + 1)) <<  8) + \
						((uint32_t)(*((x) + 2)) << 16) + \
						((uint32_t)(*((x) + 3)) << 24)

#define BYTESTOINT64(x) ((uint64_t)(*((x) + 0)) <<  0) + \
						((uint64_t)(*((x) + 1)) <<  8) + \
						((uint64_t)(*((x) + 2)) << 16) + \
						((uint64_t)(*((x) + 3)) << 24) + \
						((uint64_t)(*((x) + 4)) << 32) + \
						((uint64_t)(*((x) + 5)) << 40) + \
						((uint64_t)(*((x) + 6)) << 48) + \
						((uint64_t)(*((x) + 7)) << 56)
