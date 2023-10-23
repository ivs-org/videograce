/**
 * RTCPPacket.h - Contains RTCPPacket header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Transport/IPacket.h>

namespace Transport
{

class RTCPPacket : public IPacket
{
public:
	RTCPPacket();
	~RTCPPacket();

#define RTP_SEQ_MOD (1<<16)
#define RTP_MAX_SDES 255      /* maximum text length for SDES */

	enum rtcp_type_t
	{
		RTCP_SR = 200,
		RTCP_RR = 201,
		RTCP_SDES = 202,
		RTCP_BYE = 203,
		RTCP_APP = 204
	};

	enum rtcp_sdes_type_t
	{
		RTCP_SDES_END = 0,
		RTCP_SDES_CNAME = 1,
		RTCP_SDES_NAME = 2,
		RTCP_SDES_EMAIL = 3,
		RTCP_SDES_PHONE = 4,
		RTCP_SDES_LOC = 5,
		RTCP_SDES_TOOL = 6,
		RTCP_SDES_NOTE = 7,
		RTCP_SDES_PRIV = 8
	};

	/// RTCP common header word
	struct rtcp_common_t
	{
		unsigned int version : 2;   /* protocol version */
		unsigned int p : 1;         /* padding flag */
		unsigned int count : 5;     /* varies by packet type */
		unsigned int pt : 8;        /* RTCP packet type */
		uint16_t length;            /* pkt len in words, w/o this word */

		rtcp_common_t()
			: version(2), p(0), count(0), pt(RTCP_SR), length(0)
		{}
	};
	
	rtcp_common_t rtcp_common;

/// Big-endian mask for version, padding bit and packet type pair
#define RTCP_VALID_MASK (0xc000 | 0x2000 | 0xfe)
#define RTCP_VALID_VALUE ((RTP_VERSION << 14) | RTCP_SR)

	/// Reception report block
	struct rtcp_rr_t
	{
		uint32_t ssrc;             /* data source being reported */
		unsigned int fraction : 8;  /* fraction lost since last SR/RR */
		int lost : 24;              /* cumul. no. pkts lost (signed!) */
		uint32_t last_seq;         /* extended last seq. no. received */
		uint32_t jitter;           /* interarrival jitter */
		uint32_t lsr;              /* last SR packet from this source */
		uint32_t dlsr;             /* delay since last SR packet */
	};

	/// SDES item
	struct rtcp_sdes_item_t
	{
		uint8_t type;              /* type of item (rtcp_sdes_type_t) */
		uint8_t length;            /* length of item (in octets) */
		char data[1];              /* text, not null-terminated */
	};

	enum AppMessageType
	{
		amtUndefined = 0,

		amtForceKeyFrame,
		amtStat,
		amtReduceComplexity,
		amtSetFrameRate,

		amtUDPTest,

		amtRemoteControl
	};

	enum RemoteControlAction
	{
		rcaMove,
		rcaLeftUp,
		rcaLeftDown,
		rcaCenterUp,
		rcaCenterDown,
		rcaRightUp,
		rcaRightDown,
		rcaLeftDblClick,
		rcaRightDblClick,
		rcaWheel,
		rcaKeyUp,
		rcaKeyDown
	};

	/// One RTCP packet
	struct rtcp_t
	{
		rtcp_common_t common;     /* common header */
		union {
			/* sender report (SR) */
			struct {
				uint32_t ssrc;     /* sender generating this report */
				uint32_t ntp_sec;  /* NTP timestamp */
				uint32_t ntp_frac;
				uint32_t rtp_ts;   /* RTP timestamp */
				uint32_t psent;    /* packets sent */
				uint32_t osent;    /* octets sent */
				rtcp_rr_t rr[1];  /* variable-length list */
			} sr;

			/* reception report (RR) */
			struct {
				uint32_t ssrc;     /* receiver generating this report */
				rtcp_rr_t rr[1];  /* variable-length list */
			} rr;

			/* source description (SDES) */
			struct rtcp_sdes {
				uint32_t src;      /* first SSRC/CSRC */
				rtcp_sdes_item_t item[1]; /* list of SDES items */
			} sdes;

			/* BYE */
			struct {
				uint32_t src[1];   /* list of sources */
					/* can't express trailing text for reason */
			} bye;

			/* APP */
			struct {
				AppMessageType messageType;
				uint32_t ssrc;
				uint8_t payload[8];
			} app;
		} r;
	};

	/// Per-source state information
	struct source {
		uint16_t max_seq;        /* highest seq. number seen */
		uint32_t cycles;         /* shifted count of seq. number cycles */
		uint32_t base_seq;       /* base seq number */
		uint32_t bad_seq;        /* last 'bad' seq number + 1 */
		uint32_t probation;      /* sequ. packets till source is valid */
		uint32_t received;       /* packets received */
		uint32_t expected_prior; /* packet expected at last interval */
		uint32_t received_prior; /* packet received at last interval */
		uint32_t transit;        /* relative trans time for prev pkt */
		uint32_t jitter;         /* estimated jitter */
		/* ... */
	};
	
	rtcp_t rtcps[1];

	/// derived from IPacket
	virtual void Serialize(uint8_t *outputBuffer, uint32_t *size) const;
	virtual bool Parse(const uint8_t *inputData, uint32_t size);
	virtual PacketType GetType() const;
};

}
