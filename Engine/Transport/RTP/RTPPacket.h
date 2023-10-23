/**
 * RTPPacket.h - Contains RTPPacket header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Transport/IPacket.h>

namespace Transport
{

class RTPPacket : public IPacket
{
public:
	RTPPacket();
	~RTPPacket();

	struct RTPHeader
	{
		unsigned int version : 2;   /* protocol version */
		unsigned int p : 1;         /* padding flag */
		unsigned int x : 1;         /* header extension flag */
		unsigned int cc : 4;        /* CSRC count */
		unsigned int m : 1;         /* marker bit */
		unsigned int pt : 7;        /* payload type */
		unsigned int seq : 16;      /* sequence number */
		uint32_t ts;                /* timestamp */
		uint32_t ssrc;              /* synchronization source */
		uint32_t csrc[1];           /* optional CSRC list */
		unsigned int eXLength : 16; /* optional if x is 1*/
		uint32_t eX[2];             /* optional extension */

		RTPHeader()
			: version(2), p(0), x(0), cc(0), m(0), pt(96), seq(0), ts(0), ssrc(0), csrc{}, eXLength(0), eX{}
		{}
	};

	RTPHeader rtpHeader;
	const uint8_t *payload;
	uint32_t payloadSize;

	/// derived from IPacket
	virtual void Serialize(uint8_t *outputBuffer, uint32_t *size) const;
	virtual bool Parse(const uint8_t *inputData, uint32_t size);
	virtual PacketType GetType() const;
};

}
