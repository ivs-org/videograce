/**
 * IVideoEncoder.h - Contains video encoder interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Video/Resolution.h>
#include <Video/CodecType.h>
#include <cstdint>

namespace Transport
{
	class ISocket;
}

namespace Video
{

class IEncoder
{
	public:
		virtual void SetReceiver(Transport::ISocket *receiver) = 0;

		virtual void SetResolution(Resolution resolution) = 0;
		virtual void SetBitrate(int32_t bitrate) = 0;

		virtual void SetScreenContent(bool yes) = 0;

		virtual int GetBitrate() = 0;
		
		virtual void Start(CodecType type) = 0;
		virtual void Stop() = 0;
		virtual bool IsStarted() = 0;

		virtual void ForceKeyFrame(uint32_t lastRecvSeq) = 0;

		virtual ~IEncoder() {}
};
}
