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
	: impl(),
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
	sampleFreq = val;
	if (impl)
	{
		impl->SetSampleFreq(val);
	}
	return true;
}

bool Decoder::SetChannelsCount(int val)
{
	channels = val;
	if (impl)
	{
		impl->SetChannelsCount(val);
	}
	return true;
}

void Decoder::Start(CodecType type_)
{
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
	impl.reset(nullptr);
}

bool Decoder::IsStarted()
{
	if (impl)
	{
		return impl->IsStarted();
	}
	return false;
}

void Decoder::Decode(const Transport::IPacket& in, Transport::IPacket& out)
{
	if (impl)
	{
		impl->Decode(in, out);
	}
}

void Decoder::Send(const Transport::IPacket &packet, const Transport::Address *)
{
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
