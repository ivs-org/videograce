/**
 * VP8RTPCollector.cpp - Contains vp8 rtp unpacker impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <Video/VP8RTPCollector.h>
#include <Video/SplittedPacketSize.h>

#include <Transport/RTP/RTPPacket.h>

#include <memory.h>
#include <new>

#include <Common/BitHelpers.h>
#include <Common/CRC32.h>

#include <Common/Common.h>

namespace Video
{

static const uint32_t UNPACKER_BUFFER_SIZE = 1024 * 1024;

VP8RTPCollector::VP8RTPCollector()
	: receiver(nullptr),
	buffer(new uint8_t[UNPACKER_BUFFER_SIZE]),
	size(0),
	header(),
	lastPacketSeq(0), firstFramePacketSeq(0), currentFrameSeq(0),
	lastCRC32(0)
{
}

void VP8RTPCollector::Reset()
{
	header = { };
	size = 0;
	lastPacketSeq = 0;
	firstFramePacketSeq = 0;
	currentFrameSeq = 0;
	lastCRC32 = 0;
}

void VP8RTPCollector::SetReceiver(Transport::ISocket *receiver_)
{
	receiver = receiver_;
}

void VP8RTPCollector::Send(const Transport::IPacket &packet, const Transport::Address *)
{
	Process(packet);
}

inline uint8_t GetPayloadDescriptorSize(uint16_t firstTwoOctets)
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

	uint8_t size = 1; // in octets

	if (BitIsSet(firstTwoOctets, 0)) // X
	{
		++size;
		if (BitIsSet(firstTwoOctets, 9)) // I
		{
			++size; // todo check the 7 bit of "I payload"
		}
		if (BitIsSet(firstTwoOctets, 10)) // L
		{
			++size;
		}
		if (BitIsSet(firstTwoOctets, 11) || BitIsSet(firstTwoOctets, 12)) // T/K
		{
			++size;
		}
	}

	return size;
}

void VP8RTPCollector::Process(const Transport::IPacket &packet_)
{
	const Transport::RTPPacket &packet = *static_cast<const Transport::RTPPacket*>(&packet_);

	if (packet.payloadSize == 0 || /// drop the empty packets
		(lastPacketSeq != 0 && lastPacketSeq == packet.rtpHeader.seq)) /// drop the duplicates
	{
		return;
	}
	lastPacketSeq = packet.rtpHeader.seq;

	uint16_t firstTwoOctets = *reinterpret_cast<const uint16_t*>(packet.payload);
	uint8_t payloadDescriptorSize = GetPayloadDescriptorSize(firstTwoOctets);
		
	if (BitIsSet(firstTwoOctets, 3)) // S (the first packet in frame)
	{
		firstFramePacketSeq = lastPacketSeq;
		currentFrameSeq = packet.rtpHeader.eX[1];

		if (size != 0 && lastCRC32 == Common::crc32(0, buffer.get(), size)) // The next frame is coming, check the buffer
		{
			SendBuffer();
		}

		lastCRC32 = packet.rtpHeader.eX[0]; // Set the last crc for next first packet
		header = packet.rtpHeader;
		size = 0;
	}

	if (packet.rtpHeader.eX[1] == currentFrameSeq && packet.payloadSize > payloadDescriptorSize)
	{
		uint32_t dataSize = packet.payloadSize - payloadDescriptorSize;
		uint32_t pos = (lastPacketSeq - firstFramePacketSeq) * SPLITTED_PACKET_SIZE;

		if (pos + dataSize <= UNPACKER_BUFFER_SIZE)
		{
			memcpy(buffer.get() + pos, packet.payload + payloadDescriptorSize, dataSize);
			size += dataSize;
		}
		else
		{
			DBGTRACE("VP8RTPCollector the output buffer is overflow\n");
		}
	}
}

void VP8RTPCollector::SendBuffer()
{
	Transport::RTPPacket packet;

	packet.rtpHeader = header;
	packet.rtpHeader.seq = header.eX[1];

	packet.payload = buffer.get();
	packet.payloadSize = size;

	receiver->Send(packet);
}

}
