/**
 * CRC32.cpp - Contains definition of crc32 calc impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2017
 */

#include <Common/CRC32.h>
#include <ippdc.h>

namespace Common
{

uint32_t crc32(uint32_t crc, const void *buf, uint32_t size)
{
	Ipp32u crc32 = crc;
	ippsCRC32_8u((Ipp8u*)buf, size, &crc32);
	return crc32;
}

}
