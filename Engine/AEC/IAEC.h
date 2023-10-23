/**
 * IAEC.h - Contains acoustic echo canceller interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

namespace Transport
{
	class ISocket;
}

namespace AEC
{

class IAEC
{
public:
	virtual Transport::ISocket *GetMicrophoneReceiver() = 0;
	virtual Transport::ISocket *GetSpeakerReceiver() = 0;

	virtual void Start() = 0;
	virtual void Stop() = 0;

	virtual void EnableAEC(bool yes) = 0;
	virtual bool AECEnabled() = 0;

	virtual void EnableNS(bool yes) = 0;
	virtual bool NSEnabled() = 0;
	
	virtual void EnableAGC(bool yes) = 0;
	virtual bool AGCEnabled() = 0;
	
	virtual void SetMicLevel(int level) = 0;
	virtual void SetRenderLatency(int16_t latency) = 0;

protected:
	~IAEC() {}
};

}
