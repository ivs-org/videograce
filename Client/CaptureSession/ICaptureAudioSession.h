/**
 * ICaptureAudioSession.h - Contains interface of capture audio session
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <string>

#include <Audio/IAudioEncoder.h>

namespace Transport
{
	class ISocket;
}

namespace CaptureSession
{
	class ICaptureAudioSession
	{
	public:
		virtual void SetDeviceName(std::string_view name) = 0;
		virtual void EnableAEC(bool yes) = 0;
		virtual void EnableNS(bool yes) = 0;
		virtual void EnableAGC(bool yes) = 0;
        virtual void SetBitrate(int32_t bitrate) = 0;
		virtual void SetQuality(int32_t quality) = 0;
		
        virtual void SetGain(uint16_t gain) = 0;
        virtual uint16_t GetGain() const = 0;

        virtual void SetSampleFreq(int32_t freq) = 0;
        virtual int32_t GetSampleFreq() const = 0;
		
        virtual void SetRenderLatency(int32_t latency) = 0;
		
        virtual void SetMute(bool yes) = 0;
		virtual bool GetMute() const = 0;

		virtual void SetEncoderType(Audio::CodecType et) = 0;

		virtual void SetRTPParams(std::string_view addr, uint16_t rtpPort) = 0;
		virtual void SetWSMParams(std::string_view addr, std::string_view acessToken, std::string_view wsDestAddr) = 0;

		virtual std::string_view GetName() const = 0;
		virtual uint32_t GetDeviceId() const = 0;

		virtual int32_t GetBitrate() const = 0;

		virtual void SetLocalReceiver(Transport::ISocket* receiver) = 0;

		virtual void Start(uint32_t ssrc, uint32_t devcieId, std::string_view secureKey) = 0;
		virtual bool Started() const = 0;
		virtual void Stop() = 0;
	protected:
		~ICaptureAudioSession() {}
	};
}
