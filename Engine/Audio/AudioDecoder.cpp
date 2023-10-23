/**
 * AudioDecoder.cpp - Contains audio decoder implementation
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <Audio/AudioDecoder.h>
#include <Audio/OpusDecoderImpl.h>

namespace Audio
{

Decoder::Decoder()
	: mutex(),
	impl(),
	receiver(nullptr),
	type(CodecType::Undefined),
	sampleFreq(48000),
	channels(1)
{
}

Decoder::~Decoder()
{
	Decoder::Stop();
}

void Decoder::SetReceiver(Transport::ISocket *receiver_)
{
	receiver = receiver_;
	if (impl)
	{
		impl->SetReceiver(receiver);
	}
}

bool Decoder::SetSampleFreq(int val)
{
	std::lock_guard<std::mutex> lock(mutex);

	sampleFreq = val;
	if (impl)
	{
		impl->SetSampleFreq(val);
	}
	return true;
}

bool Decoder::SetChannelsCount(int val)
{
	std::lock_guard<std::mutex> lock(mutex);

	channels = val;
	if (impl)
	{
		impl->SetChannelsCount(val);
	}
	return true;
}

void Decoder::Start(CodecType type_)
{
	std::lock_guard<std::mutex> lock(mutex);

	if (!impl)
	{
		type = type_;

		switch (type)
		{
			case CodecType::Opus:
			{
				impl = std::unique_ptr<IDecoder>(new OpusDecoderImpl());
			}
			break;
			default:
			break;
		}

		if (impl)
		{
			impl->SetReceiver(receiver);
			impl->SetSampleFreq(sampleFreq);
			impl->SetChannelsCount(channels);
			impl->Start(type);
		}
	}
}

void Decoder::Stop()
{
	std::lock_guard<std::mutex> lock(mutex);

	impl.reset(nullptr);
}

bool Decoder::IsStarted()
{
	std::lock_guard<std::mutex> lock(mutex);

	if (impl)
	{
		return impl->IsStarted();
	}
	return false;
}

void Decoder::Send(const Transport::IPacket &packet, const Transport::Address *)
{
	std::lock_guard<std::mutex> lock(mutex);

	if (impl)
	{
		switch (type)
		{
		case CodecType::Opus:
			static_cast<OpusDecoderImpl*>(impl.get())->Send(packet);
			break;
		default:
			break;
		}
	}
}

}
