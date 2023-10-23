/**
 * IPacketLossCallback.h - Contains video packet loss callback
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

namespace Video
{
	class IPacketLossCallback
	{
	public:
		virtual void ForceKeyFrame(uint32_t lastRecvSeq) = 0;
	protected:
		~IPacketLossCallback() {}
	};
}
