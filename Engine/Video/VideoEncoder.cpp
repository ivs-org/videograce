/**
 * VideoEncoder.cpp - Contains video encoder implementation
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <Video/VideoEncoder.h>

#include <Video/VP8EncoderImpl.h>

namespace Video
{

Encoder::Encoder()
	: impl(),
	receiver(nullptr),
	type(CodecType::Undefined),
	resolution(Video::rHD),
	bitrate(1024),
	forceKFSeq(0),
	screenContent(false)
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

void Encoder::SetResolution(Resolution resolution_)
{
	resolution = resolution_;
	if (impl)
	{
		impl->SetResolution(resolution);
	}
}

void Encoder::SetBitrate(int bitrate_)
{
	bitrate = bitrate_;
	if (impl)
	{
		impl->SetBitrate(bitrate);
	}
}

void Encoder::SetScreenContent(bool yes)
{
	screenContent = yes;
	if (impl)
	{
		impl->SetScreenContent(yes);
	}
}

int Encoder::GetBitrate()
{
	return bitrate;
}

void Encoder::Start(CodecType type_)
{
	forceKFSeq = 0;

	if (!impl)
	{
		type = type_;

		switch (type)
		{
			case CodecType::VP8:
			{
				impl = std::make_unique<VP8EncoderImpl>();
			}
			break;
			case CodecType::H264:
			{
				//impl = new X264EncoderImpl();
			}
			break;
			case CodecType::VP9:
			{
				//impl = new VP9EncoderImpl();
			}
			break;
			default:
			break;
		}

		if (impl)
		{
			impl->SetReceiver(receiver);
			impl->SetResolution(resolution);
			impl->SetBitrate(bitrate);
			impl->SetScreenContent(screenContent);
			impl->Start(type);
		}
	}
}

void Encoder::Stop()
{
	impl.reset(nullptr);
}

bool Encoder::IsStarted()
{
	if (impl)
	{
		return impl->IsStarted();
	}
	return false;
}

void Encoder::ForceKeyFrame(uint32_t lastRecvSeq)
{
	if (lastRecvSeq == 0 || forceKFSeq != lastRecvSeq)
	{
		forceKFSeq = lastRecvSeq;

		if (impl)
		{
			impl->ForceKeyFrame(lastRecvSeq);
		}
	}
}

void Encoder::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	if (impl)
	{
		switch (type)
		{
			case CodecType::VP8:
				static_cast<VP8EncoderImpl*>(impl.get())->Send(packet_);
			break;
			case CodecType::H264:
				//static_cast<X264EncoderImpl*>(impl)->Send(packet_);
			break;
			case CodecType::VP9:
				//static_cast<VP9EncoderImpl*>(impl)->Send(packet_);
			break;
			default:
			break;
		}
	}
}

}
