/**
 * VP8RTPSplitter.h - Contains vp8 frame splitter to multiple rtp packets
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Transport/ISocket.h>

#include <Video/SplittedPacketSize.h>

#include <cstddef>
#include <memory>

namespace Video
{

class VP8RTPSplitter : public Transport::ISocket
{
public:
	VP8RTPSplitter();
	~VP8RTPSplitter();

	void Reset();

	void SetReceiver(Transport::ISocket *receiver);

	/// Derived from Transport::ISocket (input method)
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

private:
	Transport::ISocket *receiver;

	uint8_t buffer[SPLITTED_PACKET_SIZE + 1];

	uint16_t lastSeq;
};

}
