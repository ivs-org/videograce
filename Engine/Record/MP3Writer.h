/**
 * MP3Writer.h - Contains mp3 writer's interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <Transport/ISocket.h>

#include <lame.h>

namespace Recorder
{

class MP3Writer : public Transport::ISocket
{
public:
	MP3Writer();
	~MP3Writer();

	void Start(const char *fileName);
	void Stop();

	/// Derived from Transport::ISocket
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

private:
	FILE *mp3;
	lame_t lame;

	unsigned char mp3_buffer[1024 * 10];
};

}
