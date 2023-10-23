/**
 * AEC.h - Contains acoustic echo canceller header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <atomic>
#include <memory>
#include <mutex>

#include "IAEC.h"

#include <Transport/ISocket.h>

namespace webrtc
{
	class SplittingFilter;
	class IFChannelBuffer;
}

namespace AEC
{

class AEC : public IAEC
{
public:
	AEC();
	~AEC();

	void SetReceiver(Transport::ISocket *receiver);

	/// Impl of IAEC
	virtual Transport::ISocket *GetMicrophoneReceiver();
	virtual Transport::ISocket *GetSpeakerReceiver();
	virtual void Start();
	virtual void Stop();
	virtual void EnableAEC(bool yes);
	virtual bool AECEnabled();
	virtual void EnableNS(bool yes);
	virtual bool NSEnabled();
	virtual void EnableAGC(bool yes);
	virtual bool AGCEnabled();
	virtual void SetMicLevel(int level);
	virtual void SetRenderLatency(int16_t latency);

private:
	class MicrophoneReceiver : public Transport::ISocket
	{
		AEC &aec;
		bool runned;

		std::mutex mutex;

		std::unique_ptr<webrtc::SplittingFilter> splittingFilter;
		std::unique_ptr<webrtc::IFChannelBuffer> splittingFilterIn;
		std::unique_ptr<webrtc::IFChannelBuffer> splittingFilterOut;
	public:
		MicrophoneReceiver(AEC &aec_);
		virtual void Send(const Transport::IPacket &packet_, const Transport::Address *address = nullptr) final;
		void Start();
		void Stop();
	};

	class SpeakerReceiver : public Transport::ISocket
	{
		AEC &aec;
		bool runned;

		std::mutex mutex;

		std::unique_ptr<webrtc::SplittingFilter> splittingFilter;
		std::unique_ptr<webrtc::IFChannelBuffer> splittingFilterIn;
		std::unique_ptr<webrtc::IFChannelBuffer> splittingFilterOut;
	public:
		SpeakerReceiver(AEC &aec_);
		virtual void Send(const Transport::IPacket &packet_, const Transport::Address *address = nullptr) final;
		void Start();
		void Stop();
	};

	friend MicrophoneReceiver;
	friend SpeakerReceiver;

	std::atomic<bool> runned;
	std::atomic<bool> aecEnabled;
	std::atomic<bool> nsEnabled;
	std::atomic<bool> agcEnabled;

	int32_t micLevel;
	int16_t renderLatency;

	MicrophoneReceiver microphoneSource;
	SpeakerReceiver speakerSource;

	void *aecInst;
	void *nsInst;
	void *agcInst;

	Transport::ISocket *resultReceiver;
};

}
