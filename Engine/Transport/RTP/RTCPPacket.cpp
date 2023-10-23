/**
 * RTCPPacket.cpp - Contains RTCPPacket impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <Transport/RTP/RTCPPacket.h>
#include <memory.h>

#ifndef _WIN32
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif

#include <Common/BitHelpers.h>

namespace Transport
{

RTCPPacket::RTCPPacket()
	: rtcp_common(),
	rtcps{}
{
}

RTCPPacket::~RTCPPacket()
{
}

void RTCPPacket::Serialize(uint8_t *buffer, uint32_t *size) const
{
	uint8_t firstOctet = rtcp_common.count;

	SetBit(firstOctet, 7);
	ClearBit(firstOctet, 6); // version always 2

	ClearBit(firstOctet, 5);
	if (rtcp_common.p) SetBit(firstOctet, 5);
	
	*static_cast<uint8_t*>(buffer) = firstOctet;
	*static_cast<uint8_t*>(buffer + 1) = rtcp_common.pt;
	*reinterpret_cast<uint16_t*>(buffer + 2) = 1; // htons(rtcp_common.length); // Prevent errors (temporary)

	uint32_t headerSize = 4;

	for (int i = 0; i != rtcp_common.length; ++i)
	{
		switch (rtcp_common.pt)
		{
			case RTCP_SR:
				*reinterpret_cast<uint32_t*>(buffer + headerSize + 0) = htonl(rtcps[i].r.sr.ssrc);
				*reinterpret_cast<uint32_t*>(buffer + headerSize + 4) = htonl(rtcps[i].r.sr.ntp_sec);
				*reinterpret_cast<uint32_t*>(buffer + headerSize + 8) = htonl(rtcps[i].r.sr.ntp_frac);
				*reinterpret_cast<uint32_t*>(buffer + headerSize + 12) = htonl(rtcps[i].r.sr.rtp_ts);
				*reinterpret_cast<uint32_t*>(buffer + headerSize + 16) = htonl(rtcps[i].r.sr.psent);
				*reinterpret_cast<uint32_t*>(buffer + headerSize + 20) = htonl(rtcps[i].r.sr.osent);

				//rtcp_rr_t rr[1];  /* variable-length list */
				headerSize += 24;
			break;
			case RTCP_RR:
				*reinterpret_cast<uint32_t*>(buffer + headerSize + 0) = htonl(rtcps[i].r.rr.ssrc);
				headerSize += 4;
			break;
			case RTCP_APP:
				*reinterpret_cast<uint32_t*>(buffer + headerSize + 0) = htonl(rtcps[i].r.app.messageType);
				*reinterpret_cast<uint32_t*>(buffer + headerSize + 4) = htonl(rtcps[i].r.app.ssrc);
				memcpy(buffer + headerSize + 8, rtcps[i].r.app.payload, sizeof(rtcps[i].r.app.payload));
									
				headerSize += 8 + sizeof(rtcps[i].r.app.payload);
			break;
		}
	}

	*size = headerSize;
}

bool RTCPPacket::Parse(const uint8_t *buffer, uint32_t size)
{
	uint8_t firstOctet = *(static_cast<const uint8_t*>(buffer));

	if (BitIsSet(firstOctet, 7)) SetBit(rtcp_common.version, 1); else ClearBit(rtcp_common.version, 1); // network to host order
	if (BitIsSet(firstOctet, 6)) SetBit(rtcp_common.version, 0); else ClearBit(rtcp_common.version, 0);
	if (rtcp_common.version != 2)
	{
		return false;
	}

	rtcp_common.p = BitIsSet(firstOctet, 5);

	rtcp_common.count = firstOctet;
	ClearBit(rtcp_common.count, 7);
	ClearBit(rtcp_common.count, 6);
	ClearBit(rtcp_common.count, 5);

	rtcp_common.pt = *static_cast<const uint8_t*>(buffer + 1);
	
	rtcp_common.length = ntohs(*(reinterpret_cast<const uint16_t*>(buffer + 2)));
	if (rtcp_common.length > 1) rtcp_common.length = 1; // Prevent hacking (temporary)

	uint32_t headerSize = 4;

	for (int i = 0; i != rtcp_common.length; ++i)
	{
		switch (rtcp_common.pt)
		{
			case RTCP_SR:
				rtcps[i].r.sr.ssrc     = ntohl(*reinterpret_cast<const uint32_t*>(buffer + headerSize + 0));
				rtcps[i].r.sr.ntp_sec  = ntohl(*reinterpret_cast<const uint32_t*>(buffer + headerSize + 4));
				rtcps[i].r.sr.ntp_frac = ntohl(*reinterpret_cast<const uint32_t*>(buffer + headerSize + 8));
				rtcps[i].r.sr.rtp_ts   = ntohl(*reinterpret_cast<const uint32_t*>(buffer + headerSize + 12));
				rtcps[i].r.sr.psent    = ntohl(*reinterpret_cast<const uint32_t*>(buffer + headerSize + 16));
				rtcps[i].r.sr.osent    = ntohl(*reinterpret_cast<const uint32_t*>(buffer + headerSize + 20));

				//rtcp_rr_t rr[1];  /* variable-length list */
				headerSize += 24;
			break;
			case RTCP_RR:
				rtcps[i].r.rr.ssrc = ntohl(*reinterpret_cast<const uint32_t*>(buffer + headerSize + 0));
				headerSize += 4;
			break;
			case RTCP_APP:
				rtcps[i].r.app.messageType = (AppMessageType)ntohl(*reinterpret_cast<const uint32_t*>(buffer + headerSize + 0));
				rtcps[i].r.app.ssrc        = ntohl(*reinterpret_cast<const uint32_t*>(buffer + headerSize + 4));
				memcpy(rtcps[i].r.app.payload, reinterpret_cast<const uint32_t*>(buffer + headerSize + 8), sizeof(rtcps[i].r.app.payload));

				headerSize += 8 + sizeof(rtcps[i].r.app.payload);
			break;
		}
	}

	if (headerSize > size) // prevent hacking and overflow
	{
		return false;
	}

	return true;
}

PacketType RTCPPacket::GetType() const
{
	return PacketType::RTCP;
}

}
