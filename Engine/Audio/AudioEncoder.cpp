/**
 * AudioEncoder.cpp - Contains audio encoder implementation
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <Audio/AudioEncoder.h>
#include <Audio/OpusEncoderImpl.h>

namespace Audio
{

Encoder::Encoder()
	: impl(),
	receiver(nullptr),
	type(CodecType::Undefined),
	sampleFreq(48000),
	quality(10),
	bitrate(30),
	packetLoss(0)
{
}

Encoder::~Encoder()
{
	Encoder::Stop();
}

void Encoder::SetReceiver(Transport::ISocket *receiver_)
{
	receiver = receiver_;
	if (impl)
	{
		impl->SetReceiver(receiver);
	}
}

void Encoder::SetQuality(int32_t val)
{
	quality = val;
	if (impl)
	{
		impl->SetQuality(val);
	}
}

void Encoder::SetBitrate(int32_t bitrate_)
{
	bitrate = bitrate_;
	if (impl)
	{
		impl->SetBitrate(bitrate);
	}
}

int32_t Encoder::GetBitrate() const
{
	return bitrate;
}

void Encoder::Start(CodecType type_, uint32_t ssrc)
{
	if (!impl)
	{
		type = type_;

		switch (type)
		{
			case CodecType::Opus:
			{
				impl = std::make_unique<OpusEncoderImpl>();
			}
			break;
			default:
			break;
		}

		if (impl)
		{
			impl->SetReceiver(receiver);
			impl->SetQuality(quality);
			impl->SetBitrate(bitrate);
			impl->Start(type, ssrc);
		}
	}
}

void Encoder::Stop()
{
	impl.reset(nullptr);
}

bool Encoder::IsStarted() const
{
	if (impl)
	{
		return impl->IsStarted();
	}
	return false;
}

void Encoder::SetPacketLoss(int32_t val)
{
	packetLoss = val;
	if (impl)
	{
		impl->SetPacketLoss(val);
	}
}

void Encoder::Send(const Transport::IPacket &packet, const Transport::Address *)
{
	if (impl)
	{
		switch (type)
		{
		case CodecType::Opus:
			static_cast<OpusEncoderImpl*>(impl.get())->Send(packet);
			break;
		default:
			break;
		}
	}
}

}
