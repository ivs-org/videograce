/**
 * OpusDecoderImpl.cpp - Contains impl of opus decoder impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <Audio/OpusDecoderImpl.h>

#include <Common/Common.h>

#include <Transport/RTP/RTPPayloadType.h>

namespace Audio
{

OpusDecoderImpl::OpusDecoderImpl()
	: receiver(nullptr),
	runned(false),
	sample_freq(48000),
	channels(1),
	lastSeq(0),
	produceBuffer(),
	opusDecoder(nullptr)
{
}

OpusDecoderImpl::~OpusDecoderImpl()
{
	OpusDecoderImpl::Stop();
}

void OpusDecoderImpl::SetReceiver(Transport::ISocket *receiver_)
{
	receiver = receiver_;
}

bool OpusDecoderImpl::SetSampleFreq(int val)
{
	if (runned)
	{
		Stop();
		sample_freq = val;
		Start(CodecType::Opus);
	}
	else
	{
		sample_freq = val;
	}
	return true;
}

bool OpusDecoderImpl::SetChannelsCount(int channelsCount)
{
	if (runned)
	{
		Stop();
		channels = channelsCount;
		Start(CodecType::Opus);
	}
	else
	{
		channels = channelsCount;
	}
	return true;
}

void OpusDecoderImpl::Start(CodecType)
{
	if (runned)
	{
		return;
	}

	lastSeq = 0;

	try
	{
		produceBuffer = std::make_unique<uint8_t[]>((sample_freq / 100) * channels * 2 * 4 * 1000); // 40 * 1000 ms 48 hz mono frame
	}
	catch (std::bad_alloc &e)
	{
		return;
	}

	int opus_err = 0;
	opusDecoder = opus_decoder_create(sample_freq, channels, &opus_err);
	if (opus_err != OPUS_OK)
	{
		return;
	}

	runned = true;
}

void OpusDecoderImpl::Stop()
{
	runned = false;

	if (opusDecoder)
	{
		opus_decoder_destroy(opusDecoder);
		opusDecoder = nullptr;
	}

	produceBuffer.reset(nullptr);	
}

bool OpusDecoderImpl::IsStarted()
{
	return runned;
}

void OpusDecoderImpl::Decode(const Transport::IPacket& in_, Transport::IPacket& out_)
{
	const Transport::RTPPacket& in = *static_cast<const Transport::RTPPacket*>(&in_);

	if (lastSeq == in.rtpHeader.seq) // Drop duplicate
	{
		return;
	}

	int32_t frameSize = (sample_freq / 100) * channels * 2 * 4;
	int outframeSize = opus_decode(opusDecoder, in.payload, in.payloadSize, reinterpret_cast<opus_int16*>(produceBuffer.get()), frameSize, 0);

	if (outframeSize > 0)
	{
		int32_t outSize = outframeSize * channels * sizeof(opus_int16);

		auto& out = *static_cast<Transport::RTPPacket*>(&out_);
		out.rtpHeader = in.rtpHeader;
		out.rtpHeader.pt = static_cast<uint8_t>(Transport::RTPPayloadType::ptPCM);
		out.payload = produceBuffer.get();
		out.payloadSize = frameSize;
	}

	lastSeq = in.rtpHeader.seq;
}

void OpusDecoderImpl::Send(const Transport::IPacket &in, const Transport::Address *)
{
	if (!runned)
	{
		return;
	}
	
	Transport::RTPPacket out;

	Decode(in, out);

	if (out.payloadSize > 0)
	{
		receiver->Send(out);
	}
}

}
