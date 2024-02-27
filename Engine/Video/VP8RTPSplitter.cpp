/**
 * VP8RTPSplitter.cpp - Contains vp8 rtp splitter impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <Video/VP8RTPSplitter.h>
#include <Transport/RTP/RTPPacket.h>

#include <Common/BitHelpers.h>
#include <Common/CRC32.h>
#include <Common/ShortSleep.h>

#include <memory.h>

#include <thread>

namespace Video
{

VP8RTPSplitter::VP8RTPSplitter()
	: receiver(nullptr),
	buffer(),
	lastSeq(0)
{
}

VP8RTPSplitter::~VP8RTPSplitter()
{
}

void VP8RTPSplitter::Reset()
{
	lastSeq = 0;
}

void VP8RTPSplitter::SetReceiver(Transport::ISocket *receiver_)
{
	receiver = receiver_;
}

void VP8RTPSplitter::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	// VP8 payload descriptor.
	//       0
	//       0 1 2 3 4 5 6 7 8
	//      +-+-+-+-+-+-+-+-+-+
	//      |X|R|N|S| PART_ID |
	//      +-+-+-+-+-+-+-+-+-+
	// X:   |I|L|T|K| RSV     | (mandatory if any of the below are used)
	//      +-+-+-+-+-+-+-+-+-+
	// I:   |PictureID (8/16b)| (optional)
	//      +-+-+-+-+-+-+-+-+-+
	// L:   |   TL0PIC_IDX    | (optional)
	//      +-+-+-+-+-+-+-+-+-+
	// T/K: |TID:Y|  KEYIDX   | (optional)
	//      +-+-+-+-+-+-+-+-+-+

	using namespace std::chrono;

	const Transport::RTPPacket &inputPacket = *static_cast<const Transport::RTPPacket*>(&packet_);

	SetBit(buffer[0], 3); /// Set the S flag

	uint32_t crc32data = Common::crc32(0, inputPacket.payload, inputPacket.payloadSize);

	for (uint32_t pos = 0; pos != inputPacket.payloadSize;)
	{
		auto start = high_resolution_clock::now();

		Transport::RTPPacket packet;
		packet.rtpHeader = inputPacket.rtpHeader;
		packet.rtpHeader.seq = ++lastSeq;
		packet.rtpHeader.x = 1;
		packet.rtpHeader.eXLength = 2;
		packet.rtpHeader.eX[0] = crc32data;
		packet.rtpHeader.eX[1] = inputPacket.rtpHeader.seq;

		uint32_t size = (inputPacket.payloadSize - pos > SPLITTED_PACKET_SIZE) ? SPLITTED_PACKET_SIZE : inputPacket.payloadSize - pos;

		memcpy(buffer + 1, inputPacket.payload + pos, size);

		packet.payload = buffer;
		packet.payloadSize = size + 1;

		receiver->Send(packet);

		/// Preventing packet loss
		Common::ShortSleep();
		
		pos += size;
		
		buffer[0] = 0; /// Clear the S flag
	}
}

}
