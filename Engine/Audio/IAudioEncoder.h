/**
 * IAudioEncoder.h - Contains audio encoder interface
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

class IEncoder
{
public:
	virtual void SetReceiver(Transport::ISocket *receiver) = 0;

	virtual void SetQuality(int32_t val) = 0;
	virtual void SetBitrate(int32_t bitrate) = 0;

	virtual int32_t GetBitrate() const = 0;

	virtual void Start(CodecType type, uint32_t ssrc) = 0;
	virtual void Stop() = 0;
	virtual bool IsStarted() const = 0;

	virtual void SetPacketLoss(int32_t val) = 0;

	virtual ~IEncoder() {}
};

}
