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
	: mutex(),
	impl(),
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
	std::lock_guard<std::mutex> lock(mutex);

	receiver = receiver_;
	if (impl)
	{
		impl->SetReceiver(receiver);
	}
}

void Encoder::SetQuality(int32_t val)
{
	std::lock_guard<std::mutex> lock(mutex);

	quality = val;
	if (impl)
	{
		impl->SetQuality(val);
	}
}

void Encoder::SetBitrate(int32_t bitrate_)
{
	std::lock_guard<std::mutex> lock(mutex);

	bitrate = bitrate_;
	if (impl)
	{
		impl->SetBitrate(bitrate);
	}
}

int32_t Encoder::GetBitrate() const
{
	std::lock_guard<std::mutex> lock(mutex);

	return bitrate;
}

void Encoder::Start(CodecType type_, uint32_t ssrc)
{
	std::lock_guard<std::mutex> lock(mutex);

	if (!impl)
	{
		type = type_;

		switch (type)
		{
			case CodecType::Opus:
			{
				impl = std::unique_ptr<IEncoder>(new OpusEncoderImpl());
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
	std::lock_guard<std::mutex> lock(mutex);

	impl.reset(nullptr);
}

bool Encoder::IsStarted() const
{
	std::lock_guard<std::mutex> lock(mutex);

	if (impl)
	{
		return impl->IsStarted();
	}
	return false;
}

void Encoder::SetPacketLoss(int32_t val)
{
	std::lock_guard<std::mutex> lock(mutex);

	packetLoss = val;
	if (impl)
	{
		impl->SetPacketLoss(val);
	}
}

void Encoder::Send(const Transport::IPacket &packet, const Transport::Address *)
{
	std::lock_guard<std::mutex> lock(mutex);

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
