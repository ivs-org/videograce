/**
 * CaptureAudioSession.h - Contains impl interface of capture audio session
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2024
 * 
 *                                                                           ,-> [Encoder] -> [Encryptor] -> [NetSocket]
 *                                                    ,-> [SilentSplitter] -<
 * [Microphone] -> [AEC] -> [LocalReceiverSplitter] -<                       '-> [SilentDetector]
 *                   ^                                '-> [LocalRenderer]
 *                   '- [AudioRenderer]
 */

#pragma once

#include <memory>

#include "ICaptureAudioSession.h"

#include <Transport/RTPSocket.h>
#include <Transport/SocketSplitter.h>

#include <Transport/WSM/WSMSocket.h>

#include <Common/TimeMeter.h>

#include <Microphone/Microphone.h>
#include <Audio/AudioEncoder.h>
#include <AudioRenderer/AudioRenderer.h>
#include <Audio/SilentDetector.h>
#include <AEC/AEC.h>

#include <Crypto/Encryptor.h>

#include <spdlog/spdlog.h>

namespace CaptureSession
{
	class CaptureAudioSession : public ICaptureAudioSession, public Transport::ISocket, public Audio::SilentDetectorCallback
	{
	public:
		CaptureAudioSession(Common::TimeMeter &timeMeter);
		~CaptureAudioSession();
		
		void SetAudioRenderer(AudioRenderer::AudioRenderer *audioRenderer);
        void SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback);

		/// derived from ICaptureAudioSession
		virtual void SetDeviceName(std::string_view name);
		virtual void EnableAEC(bool yes);
		virtual void EnableNS(bool yes);
		virtual void EnableAGC(bool yes);
		virtual void SetBitrate(int32_t bitrate);
		virtual void SetQuality(int32_t quality);
		virtual void SetGain(uint16_t gain);
        virtual uint16_t GetGain() const;
        virtual void SetSampleFreq(int32_t freq);
        virtual int32_t GetSampleFreq() const;
		virtual void SetRenderLatency(int32_t latency);
		virtual void SetMute(bool yes);
		virtual bool GetMute() const;
		virtual void SetEncoderType(Audio::CodecType et);
		virtual void SetRTPParams(std::string_view addr, uint16_t rtpPort);
		virtual void SetWSMParams(std::string_view addr, std::string_view accessToken, std::string_view wsDestAddr);
		virtual std::string_view GetName() const;
		virtual uint32_t GetDeviceId() const;
		virtual int32_t GetBitrate() const;
		virtual void SetLocalReceiver(Transport::ISocket* receiver);
		virtual void Start(uint32_t ssrc, uint32_t deviceId, std::string_view secureKey);
		virtual bool Started() const;
		virtual void Stop();

		/// derived from Transport::ISocket (receive RTCP only)
		virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr);
	private:
        Client::DeviceNotifyCallback deviceNotifyCallback;
		AudioRenderer::AudioRenderer *audioRenderer;
		
		Transport::RTPSocket rtpSocket;
		Transport::WSMSocket wsmSocket;
		
		Crypto::Encryptor encryptor;
		Audio::Encoder encoder;
		Audio::SilentDetector silentDetector;
		Transport::SocketSplitter silentSplitter;
		Transport::SocketSplitter localReceiverSplitter;
		AEC::AEC aec;
		MicrophoneNS::Microphone microphone;

		bool runned;

		uint32_t ssrc, deviceId;

		std::string name;
		Audio::CodecType encoderType;

		std::string wsAddr, accessToken, wsDestAddr;
		
		std::shared_ptr<spdlog::logger> sysLog, errLog;

		/// Derived from Audio::SilentDetectorCallback
		virtual void SilentChanged(Audio::SilentMode silentMode) final;
	};

	typedef std::shared_ptr<CaptureAudioSession> CaptureAudioSessionPtr_t;
}
