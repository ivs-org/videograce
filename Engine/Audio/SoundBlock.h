/**
 * SoundBlock.h - Contains soundblock
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <memory.h>
#include <cstdint>

namespace Audio
{

struct soundblock_t
{
	uint8_t data[3840];
	uint16_t size;
	uint32_t seq;
	uint32_t ts;

	soundblock_t()
		: size(0), seq(0), ts(0)
	{
		memset(data, 0, sizeof(data));
	}

	soundblock_t(const uint8_t *data_, uint16_t size_, uint32_t seq_, uint32_t ts_)
		: size(size_ <= sizeof(data) ? size_ : sizeof(data)), seq(seq_), ts(ts_)
	{
		memcpy(data, data_, size);
	}
};

}
