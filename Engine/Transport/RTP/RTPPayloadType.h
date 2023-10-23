/**
 * RTPPayloadType.h - Contains RTP payload type enum
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

namespace Transport
{

enum class RTPPayloadType : int8_t
{
	ptUndefined = 0,

	ptPCM  = 8,
	ptVP8  = 96,
	ptOpus = 111,
	ptText = 120,
	ptData = 122
};

}
