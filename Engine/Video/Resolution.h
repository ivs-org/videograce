/**
 * Resolution.h - Contains video resolutions type and methods
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2017
 */

#pragma once

#include <cstdint>

namespace Video
{
	typedef uint32_t Resolution;

	struct ResolutionValues
	{
		uint16_t width;
		uint16_t height;

		ResolutionValues()
			: width(0), height(0)
		{}
		ResolutionValues(uint16_t width_, uint16_t height_)
			: width(width_), height(height_)
		{}
	};

	ResolutionValues GetValues(Resolution resolution);
	Resolution GetResolution(ResolutionValues values);

	static const Resolution rUndefined = 0;
	static const Resolution rQVGA      = GetResolution(ResolutionValues(320, 240));
	static const Resolution rCIF       = GetResolution(ResolutionValues(352, 288));
	static const Resolution rVGA       = GetResolution(ResolutionValues(640, 480));
	static const Resolution r4CIF      = GetResolution(ResolutionValues(704, 576));
	static const Resolution rHD        = GetResolution(ResolutionValues(1280, 720));
	static const Resolution rHD1080p   = GetResolution(ResolutionValues(1920, 1080));
	static const Resolution rUHD4K     = GetResolution(ResolutionValues(3840, 2160));
}
