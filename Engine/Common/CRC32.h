/**
 * CRC32.h - Contains definition of crc32 calc method
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2017
 */

#pragma once

#include <cstdint>

namespace Common
{

uint32_t crc32(uint32_t crc, const void *buf, uint32_t size);

}
