/**
 * ColorSpace.h - Contains color spaces enumeration
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016 - 2017
 */

#pragma once

namespace Video
{

enum class ColorSpace
{
	MJPG,
	I420,
	YUY2,
	UYVU,
	RGB24,
	RGB32,
		
	Unsupported
};

}
