/**
 * VideoDecoder.cpp - Contains video decoder implementation
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <Video/VideoDecoder.h>
#include <Video/VP8DecoderImpl.h>

#include <Transport/RTP/RTPPacket.h>

#include <memory.h>

namespace Video
{

Decoder::Decoder()
	: mutex(),
	impl(),
	receiver(nullptr),
	callback(nullptr),
	type(CodecType::Undefined),
	outputType(Video::ColorSpace::RGB24),
	resolution(Video::rVGA)
{
}

Decoder::~Decoder()
{
	Decoder::Stop();
}

void Decoder::SetReceiver(Transport::ISocket *receiver_)
{
	std::lock_guard<std::mutex> lock(mutex);

	receiver = receiver_;
	if (impl)
	{
		impl->SetReceiver(receiver);
	}
}

void Decoder::SetCallback(IPacketLossCallback *callback_)
{
	std::lock_guard<std::mutex> lock(mutex);

	callback = callback_;
	if (impl)
	{
		impl->SetCallback(callback);
	}
}

bool Decoder::SetResolution(Resolution resolution_)
{
	std::lock_guard<std::mutex> lock(mutex);

	resolution = resolution_;
	if (impl)
	{
		impl->SetResolution(resolution);
	}
	return true;
}

void Decoder::Start(CodecType type_, Video::ColorSpace outputType_)
{
	std::lock_guard<std::mutex> lock(mutex);

	if (!impl)
	{
		type = type_;
		outputType = outputType_;

		switch (type)
		{
			case CodecType::VP8:
			{
				impl = std::unique_ptr<IDecoder>(new VP8DecoderImpl());
			}
			break;
			default:
			break;
		}

		if (impl)
		{
			impl->SetReceiver(receiver);
			impl->SetCallback(callback);
			impl->SetResolution(resolution);
			impl->Start(type, outputType);
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
		case CodecType::VP8:
			static_cast<VP8DecoderImpl*>(impl.get())->Send(packet);
			break;
		default:
			break;
		}
	}
}

}
