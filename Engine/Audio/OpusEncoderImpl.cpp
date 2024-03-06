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
	quality(10),
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
		produceBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[BUFFER_SIZE]);
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

void OpusEncoderImpl::EncodeFrame(const uint8_t* data, int32_t len, const Transport::RTPPacket::RTPHeader& header)
{
	int32_t frameSize = len / (1 * sizeof(opus_int16));
    int32_t compressedSize = opus_encode(opusEncoder, (const opus_int16*)data, frameSize, produceBuffer.get(), BUFFER_SIZE);

	if (compressedSize > 0)
	{
		Transport::RTPPacket packet;

		packet.rtpHeader = header;
		packet.rtpHeader.pt = static_cast<uint8_t>(Transport::RTPPayloadType::ptOpus);
		packet.rtpHeader.x = 1;
		packet.rtpHeader.eXLength = 1;
		packet.rtpHeader.eX[0] = Common::crc32(0, produceBuffer.get(), compressedSize);

		packet.payload = produceBuffer.get();
		packet.payloadSize = compressedSize;

		receiver->Send(packet);
	}
}

void OpusEncoderImpl::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	if (!runned)
	{
		return;
	}
	const Transport::RTPPacket &packet = *static_cast<const Transport::RTPPacket*>(&packet_);

	EncodeFrame(packet.payload, packet.payloadSize, packet.rtpHeader);
}

}
