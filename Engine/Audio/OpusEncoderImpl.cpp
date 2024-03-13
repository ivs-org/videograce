/**
 * OpusEncoderImpl.cpp - Contains impl of opus encoder impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <Audio/OpusEncoderImpl.h>

#include <Common/Common.h>

#include <Transport/RTP/RTPPacket.h>
#include <Transport/RTP/RTPPayloadType.h>

#include <Common/CRC32.h>

namespace Audio
{

static const auto BUFFER_SIZE = 1024 * 10;

OpusEncoderImpl::OpusEncoderImpl()
	: receiver(nullptr),
	runned(false),
	sampleFreq(48000),
	quality(0),
	bitrate(30),
	packetLoss(0),
	produceBuffer(),
	opusEncoder()
{
}

OpusEncoderImpl::~OpusEncoderImpl()
{
	OpusEncoderImpl::Stop();
}

void OpusEncoderImpl::SetReceiver(Transport::ISocket *receiver_)
{
	receiver = receiver_;
}

void OpusEncoderImpl::SetQuality(int32_t val)
{
	quality = val;
	if (runned)
	{
		opus_encoder_ctl(opusEncoder, OPUS_SET_COMPLEXITY(val));
	}
}

void OpusEncoderImpl::SetBitrate(int32_t bitrate_)
{
	bitrate = bitrate_;
	if (runned)
	{
		opus_encoder_ctl(opusEncoder, OPUS_SET_BITRATE(bitrate * 1000));
	}
}

void OpusEncoderImpl::SetSampleFreq(int32_t freq)
{
	sampleFreq = freq;
	if (runned)
	{
		Stop();
		Start(CodecType::Opus);
	}
}

int32_t OpusEncoderImpl::GetBitrate() const
{
	return bitrate;
}

void OpusEncoderImpl::Start(CodecType)
{
	if (runned)
	{
		return;
	}
	
	try
	{
		produceBuffer = std::make_unique<uint8_t[]>(BUFFER_SIZE);
	}
	catch (std::bad_alloc &e)
	{
		return;
	}

	int opus_err = 0;
	opusEncoder = opus_encoder_create(sampleFreq, 1, OPUS_APPLICATION_VOIP, &opus_err);
	if (opus_err != OPUS_OK)
	{
		return;
	}

	opus_err = opus_encoder_ctl(opusEncoder, OPUS_SET_BITRATE(bitrate * 1000));
	if (opus_err != OPUS_OK)
	{
		return;
	}

	/*opus_err = opus_encoder_ctl(opusEncoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
	if (opus_err != OPUS_OK)
	{
		return;
	}*/

	opus_err = opus_encoder_ctl(opusEncoder, OPUS_SET_INBAND_FEC(1));
	if (opus_err != OPUS_OK)
	{
		return;
	}

	opus_err = opus_encoder_ctl(opusEncoder, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND));
	if (opus_err != OPUS_OK)
	{
		return;
	}

	opus_err = opus_encoder_ctl(opusEncoder, OPUS_SET_PACKET_LOSS_PERC(packetLoss));
	if (opus_err != OPUS_OK)
	{
		return;
	}

	opus_err = opus_encoder_ctl(opusEncoder, OPUS_SET_COMPLEXITY(quality));
	if (opus_err != OPUS_OK)
	{
		return;
	}

	runned = true;
}

void OpusEncoderImpl::Stop()
{
	runned = false;

	if (opusEncoder)
	{
		opus_encoder_destroy(opusEncoder);
		opusEncoder = nullptr;
	}

	produceBuffer.reset(nullptr);
}

bool OpusEncoderImpl::IsStarted() const
{
	return runned;
}

void OpusEncoderImpl::SetPacketLoss(int val)
{
	packetLoss = val;
	if (runned)
	{
		opus_encoder_ctl(opusEncoder, OPUS_SET_PACKET_LOSS_PERC(val));
	}
}

void OpusEncoderImpl::Encode(const Transport::IPacket& in_, Transport::IPacket& out_)
{
	const auto& in = *static_cast<const Transport::RTPPacket*>(&in_);

	int32_t frameSize = in.payloadSize / (1 * sizeof(opus_int16));
	int32_t compressedSize = opus_encode(opusEncoder, (const opus_int16*)in.payload, frameSize, produceBuffer.get(), BUFFER_SIZE);

	auto& out = *static_cast<Transport::RTPPacket*>(&out_);
	out.rtpHeader = in.rtpHeader;
	out.rtpHeader.pt = static_cast<uint8_t>(Transport::RTPPayloadType::ptOpus);
	out.rtpHeader.x = 1;
	out.rtpHeader.eXLength = 1;
	out.rtpHeader.eX[0] = Common::crc32(0, produceBuffer.get(), compressedSize);
	out.payload = produceBuffer.get();
	out.payloadSize = compressedSize;
}

void OpusEncoderImpl::Send(const Transport::IPacket &in, const Transport::Address *)
{
	if (!runned)
	{
		return;
	}
	
	Transport::RTPPacket out;

	Encode(in, out);

	if (out.payloadSize > 0)
	{
		receiver->Send(out);
	}
}

}
