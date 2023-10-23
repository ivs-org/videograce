/**
 * RTPPacket.cpp - Contains RTPPacket impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <Transport/RTP/RTPPacket.h>
#include <memory.h>

#ifndef _WIN32
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif

#include <Common/BitHelpers.h>

namespace Transport
{

RTPPacket::RTPPacket()
	: rtpHeader(),
	payload(nullptr),
	payloadSize(0)
{
}

RTPPacket::~RTPPacket()
{
}

void RTPPacket::Serialize(uint8_t *buffer, uint32_t *size) const
{
	/* RTP header
	  0                   1                   2                   3
	  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	  |V=2|P|X|  CC   |M|     PT      |       sequence number         |
	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	  |                           timestamp                           |
	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	  |           synchronization source (SSRC) identifier            |
	  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
	  |            contributing source (CSRC) identifiers             |
	  |                             ....                              |\
	  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
	  |      defined by profile       |           length              |
	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	  |                        header extension                       |
	  |                             ....                              |
	*/
	
	uint8_t firstOctet = rtpHeader.cc;

	SetBit(firstOctet, 7);
	ClearBit(firstOctet, 6); // version always 2

	ClearBit(firstOctet, 5);
	ClearBit(firstOctet, 4);
	if (rtpHeader.p) SetBit(firstOctet, 5);
	if (rtpHeader.x) SetBit(firstOctet, 4);
	
	uint8_t secondOctet = rtpHeader.pt;
	ClearBit(secondOctet, 7);
	if (rtpHeader.m) SetBit(secondOctet, 7);
	
	*static_cast<uint8_t*>(buffer) = firstOctet;
	*static_cast<uint8_t*>(buffer + 1) = secondOctet;
	*reinterpret_cast<uint16_t*>(buffer + 2) = htons(rtpHeader.seq);
	*reinterpret_cast<uint32_t*>(buffer + 4) = htonl(rtpHeader.ts);
	*reinterpret_cast<uint32_t*>(buffer + 8) = htonl(rtpHeader.ssrc);

	for (int i = 0; i != rtpHeader.cc; ++i)
	{
		*reinterpret_cast<uint32_t*>(buffer + 12 + (i * 4)) = htonl(rtpHeader.csrc[i]);
	}

	uint32_t headerSize = 12 + (rtpHeader.cc * 4);
	if (rtpHeader.x)
	{
		*reinterpret_cast<uint16_t*>(buffer + 12 + (rtpHeader.cc * 4) + 2) = htons(rtpHeader.eXLength);
		for (int i = 0; i != rtpHeader.eXLength; ++i)
		{
			*reinterpret_cast<uint32_t*>(buffer + 12 + (rtpHeader.cc * 4) + 4 + (i * 4)) = htonl(rtpHeader.eX[i]);
		}
		headerSize += 4 + (rtpHeader.eXLength * 4);
	}

	memcpy(buffer + headerSize, payload, payloadSize);
	*size = headerSize + payloadSize;
}

bool RTPPacket::Parse(const uint8_t *buffer, uint32_t size)
{
	uint8_t firstOctet = *(static_cast<const uint8_t*>(buffer));

	if (BitIsSet(firstOctet, 7)) SetBit(rtpHeader.version, 1); else ClearBit(rtpHeader.version, 1); // network to host order
	if (BitIsSet(firstOctet, 6)) SetBit(rtpHeader.version, 0); else ClearBit(rtpHeader.version, 0);
	if (rtpHeader.version != 2)
	{
		return false;
	}

	rtpHeader.p = BitIsSet(firstOctet, 5);
	rtpHeader.x = BitIsSet(firstOctet, 4);
	
	rtpHeader.cc = firstOctet;
	ClearBit(rtpHeader.cc, 7);
	ClearBit(rtpHeader.cc, 6);
	ClearBit(rtpHeader.cc, 5);
	ClearBit(rtpHeader.cc, 4);

	rtpHeader.pt = *(static_cast<const uint8_t*>(buffer + 1));;
	rtpHeader.m = BitIsSet(rtpHeader.pt, 7);
	ClearBit(rtpHeader.pt, 7);

	rtpHeader.seq = ntohs(*reinterpret_cast<const uint16_t*>(buffer + 2));
	rtpHeader.ts = ntohl(*reinterpret_cast<const uint32_t*>(buffer + 4));
	rtpHeader.ssrc = ntohl(*reinterpret_cast<const uint32_t*>(buffer + 8));

	const int csrcMaxCount = sizeof(rtpHeader.csrc) / sizeof(rtpHeader.csrc[0]);
	const int csrcCount = rtpHeader.cc > csrcMaxCount ? csrcMaxCount : rtpHeader.cc;
	for (int i = 0; i != csrcCount; ++i)
	{
		rtpHeader.csrc[i] = ntohl(*reinterpret_cast<const uint32_t*>(buffer + 12 + (i * 4)));
	}

	uint32_t headerSize = 12 + (rtpHeader.cc * 4);
	if (rtpHeader.x)
	{
		rtpHeader.eXLength = ntohs(*reinterpret_cast<const uint16_t*>(buffer + 12 + (rtpHeader.cc * 4) + 2));

		const int eXMaxCount = sizeof(rtpHeader.eX) / sizeof(rtpHeader.eX[0]);
		const int eXCount = rtpHeader.eXLength > eXMaxCount ? eXMaxCount : rtpHeader.eXLength;
		for (int i = 0; i != eXCount; ++i)
		{
			rtpHeader.eX[i] = ntohl(*reinterpret_cast<const uint32_t*>(buffer + 12 + (rtpHeader.cc * 4) + 4 + (i * 4)));
		}
		headerSize += 4 + (rtpHeader.eXLength * 4);
	}

	if (headerSize > size) // prevent hacking and overflow
	{
		return false;
	}

	payload = buffer + headerSize;
	payloadSize = size - headerSize;

	return true;
}

PacketType RTPPacket::GetType() const
{
	return PacketType::RTP;
}

}
