/**
 * VP8RTPCollector.h - Contains vp8 rtp collector header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2018
 */

#pragma once

#include <cstdint>
#include <memory>

#include <Transport/ISocket.h>
#include <Transport/RTP/RTPPacket.h>

namespace Video
{

class VP8RTPCollector : public Transport::ISocket
{
public:
	VP8RTPCollector();

	void Reset();

	void SetReceiver(Transport::ISocket *receiver);

	/// Derived from Transport::ISocket (input method)
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

private:
	Transport::ISocket *receiver;

	std::unique_ptr<uint8_t[]> buffer;
	uint32_t size;

	Transport::RTPPacket::RTPHeader header;

	uint16_t lastPacketSeq, firstFramePacketSeq, currentFrameSeq;

	uint32_t lastCRC32;

	void Process(const Transport::IPacket &packet);

	void SendBuffer();
};

}
