/**
 * AudioEncoder.h - Contains audio encoder impl interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <memory>
#include <mutex>

#include <Audio/IAudioEncoder.h>
#include <Transport/ISocket.h>

namespace Audio
{

class Encoder : public IEncoder, public Transport::ISocket
{
public:
	Encoder();
	~Encoder();

	/// Derived from IEncoder
	virtual void SetReceiver(Transport::ISocket *receiver);
	virtual void SetQuality(int32_t val);
	virtual void SetBitrate(int32_t bitrate);
	virtual int32_t GetBitrate() const;
	virtual void Start(CodecType type, uint32_t ssrc);
	virtual void Stop();
	virtual bool IsStarted() const;
	virtual void SetPacketLoss(int32_t val);

	/// Derived from Transport::ISocket (input method)
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

private:
	mutable std::mutex mutex;

	std::unique_ptr<IEncoder> impl;
	Transport::ISocket *receiver;

	CodecType type;
    int32_t sampleFreq, quality;
    int32_t bitrate;
    int32_t packetLoss;
};

}
