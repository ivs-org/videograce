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
	if (opusDecoder)
	{
		opus_decoder_destroy(opusDecoder);
		opusDecoder = nullptr;
	}

	produceBuffer.reset(nullptr);
	
	runned = false;
}

bool OpusDecoderImpl::IsStarted()
{
	return runned;
}

void OpusDecoderImpl::DecodeFrame(const uint8_t *data, uint32_t length, const Transport::RTPPacket::RTPHeader &header)
{
	int32_t frameSize = (sample_freq / 100) * channels * 2 * 4;
	int outframeSize = opus_decode(opusDecoder, data, length, reinterpret_cast<opus_int16*>(produceBuffer.get()), frameSize, 0);

	if (outframeSize > 0)
	{
		int32_t outSize = outframeSize * channels * sizeof(opus_int16);

		int32_t i = 0;
		//while (frameSize * i < outSize)
		{
			Transport::RTPPacket packet;

			packet.rtpHeader = header;
			packet.payload = produceBuffer.get();// +(frameSize * i);
			packet.payloadSize = frameSize;

			receiver->Send(packet);
			//++i;
		}
	}
}

void OpusDecoderImpl::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	if (!runned)
	{
		return;
	}
	const Transport::RTPPacket &packet = *static_cast<const Transport::RTPPacket*>(&packet_);

	if (lastSeq == packet.rtpHeader.seq) // Drop duplicate
	{
		return;
	}

	/*while (lastSeq != 0 && packet.rtpHeader.seq > lastSeq + 1) // we have a packet loss
	{
		DecodeFrame(nullptr, 0, packet.rtpHeader);

		++lastSeq;
	}*/

	DecodeFrame(packet.payload, packet.payloadSize, packet.rtpHeader);

	lastSeq = packet.rtpHeader.seq;
}

}
