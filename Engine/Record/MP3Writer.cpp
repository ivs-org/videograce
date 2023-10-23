/**
 * MP3Writer.cpp - Contains mp3 writer's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#include <Record/MP3Writer.h>

#include <Common/Common.h>
#include <Transport/RTP/RTPPacket.h>
#include <Transport/RTP/RTPPayloadType.h>

namespace Recorder
{

MP3Writer::MP3Writer()
	: mp3(),
	lame(),
	mp3_buffer()
{
}

MP3Writer::~MP3Writer()
{
}

void MP3Writer::Start(const char *fileName)
{
#ifndef _WIN32
	mp3 = fopen(fileName, "wb");
#else
	fopen_s(&mp3, fileName, "wb");
#endif

    if (!mp3)
    {
        return;
    }

	lame = lame_init();
	lame_set_in_samplerate(lame, 48000);
	lame_set_num_channels(lame, 1);
	lame_set_VBR(lame, vbr_default);
	lame_init_params(lame);
}

void MP3Writer::Stop()
{
    if (!mp3)
    {
        return;
    }

	lame_encode_flush(lame, mp3_buffer, sizeof(mp3_buffer));
	lame_close(lame);
	fclose(mp3);
}

void MP3Writer::Send(const Transport::IPacket &packet_, const Transport::Address *address)
{
    if (!mp3)
    {
        return;
    }

	const auto &packet = *static_cast<const Transport::RTPPacket*>(&packet_);
	if (packet.payloadSize == 0)
	{
		return;
	}

	auto write = lame_encode_buffer(lame, (short*)packet.payload, nullptr, packet.payloadSize / 2, mp3_buffer, sizeof(mp3_buffer));
	fwrite(mp3_buffer, write, 1, mp3);
}

}
