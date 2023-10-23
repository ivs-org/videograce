/**
 * Resolution.cpp - Contains video resolution helper impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include "Resolution.h"

namespace Video
{
	ResolutionValues GetValues(Resolution resolution)
	{
		ResolutionValues rv;

		rv.width  = *reinterpret_cast<uint16_t*>(&resolution);
		rv.height = *(reinterpret_cast<uint16_t*>(&resolution) + 1);

		return rv;
	}

	Resolution GetResolution(ResolutionValues rv)
	{
		Resolution resolution;

		*reinterpret_cast<uint16_t*>(&resolution)       = rv.width;
		*(reinterpret_cast<uint16_t*>(&resolution) + 1) = rv.height;

		return resolution;
	}
}
