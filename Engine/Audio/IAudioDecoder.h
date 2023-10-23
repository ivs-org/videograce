/**
 * IAudioDecoder.h - Contains audio decoder interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <cstdint>
#include <Audio/CodecType.h>

namespace Transport
{
	class ISocket;
}

namespace Audio
{
	class IDecoder
	{
	public:
		virtual void SetReceiver(Transport::ISocket *receiver) = 0;

		virtual bool SetSampleFreq(int val) = 0;
		virtual bool SetChannelsCount(int channelsCount) = 0;

		virtual void Start(CodecType type) = 0;
		virtual void Stop() = 0;
		virtual bool IsStarted() = 0;

		virtual ~IDecoder() {}
	};
}
