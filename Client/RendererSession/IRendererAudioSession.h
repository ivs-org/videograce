/**
 * IRendererAudioSession.h - Contains interface of renderer audio session
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <string>
#include <Audio/IAudioDecoder.h>

namespace Transport
{
	class ISocket;
}

namespace Recorder
{
	class Recorder;
}

namespace RendererSession
{
	class IRendererAudioSession
	{
	public:
		virtual void SetName(std::string_view name) = 0;
		virtual std::string_view GetName() = 0;

		virtual void SetMetadata(std::string_view metadata) = 0;
		virtual std::string_view GetMetadata() = 0;

		virtual void SetMy(bool yes) = 0;
		virtual bool GetMy() = 0;
		virtual Transport::ISocket* GetDirectReceiver() = 0;

		virtual void SetVolume(int vol) = 0;
				
		virtual void SetMute(bool yes) = 0;
		virtual bool GetMute() = 0;
		
		virtual void SetDecoderType(Audio::CodecType dt) = 0;
		
		virtual void SetRTPParams(std::string_view recvFromAddr, uint16_t recvFromRTPPort) = 0;
		virtual void SetWSMParams(std::string_view addr, std::string_view acessToken, std::string_view wsDestAddr) = 0;

		virtual void SetClientId(int64_t clientId) = 0;
		virtual int64_t GetClientId() const = 0;

		virtual uint32_t GetDeviceId() const = 0;
		virtual uint32_t GetSSRC() const = 0;
		virtual uint32_t GetAuthorSSRC() const = 0;
		virtual std::string GetSecureKey() const = 0;

		virtual void SetRecorder(Recorder::Recorder* recorder) = 0;
		
		virtual void Start(uint32_t receiverSSRC, uint32_t authorSSRC, uint32_t deviceId, std::string_view secureKey) = 0;
		virtual bool Started() const = 0;
		virtual void Stop() = 0;

		virtual void Pause() = 0;
		virtual void Resume() = 0;
	protected:
		~IRendererAudioSession() {}
	};
}
