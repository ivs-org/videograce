/**
 * IRendererVideoSession.h - Contains interface of renderer video session
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <string>

#include <Video/Resolution.h>
#include <Video/IVideoDecoder.h>
#include <Video/IPacketLossCallback.h>
#include <Proto/DeviceType.h>

#include <wui/control/i_control.hpp>

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
	class IRendererVideoSession
	{
	public:
		virtual std::shared_ptr<wui::i_control> GetControl() = 0;
        
		virtual void SetName(std::string_view name) = 0;
		virtual std::string_view GetName() = 0;

		virtual void SetDeviceType(Proto::DeviceType deviceType) = 0;
		virtual Proto::DeviceType GetDeviceType() = 0;

		virtual void SetMy(bool yes) = 0;
		virtual bool GetMy() = 0;
		virtual Transport::ISocket* GetDirectReceiver() = 0;

		virtual void SetResolution(Video::Resolution resolution) = 0;
		virtual Video::Resolution GetResolution() = 0;

		virtual void SetFrameRate(uint32_t rate) = 0;

		virtual void SetMirrorVideo(bool yes) = 0;
		virtual bool GetVideoMirrored() const = 0;

		virtual void SetDecoderType(Video::CodecType dt) = 0;
				
		virtual void SetRTPParams(std::string_view recvFromAddr, uint16_t recvFromRTPPort) = 0;
		virtual void SetWSMParams(std::string_view addr, std::string_view acessToken, std::string_view wsDestAddr) = 0;

		virtual void SetRecorder(Recorder::Recorder* recorder) = 0;
		
		virtual void SetSpeak(bool speak) = 0;
		
		virtual void SetOrder(uint32_t order) = 0;
		virtual uint32_t GetOrder() const = 0;

		virtual void SetClientId(int64_t clientId) = 0;
		virtual int64_t GetClientId() const = 0;

		virtual uint32_t GetDeviceId() const = 0;
		virtual uint32_t GetSSRC() const = 0;
		virtual uint32_t GetAuthorSSRC() const = 0;
		virtual std::string GetSecureKey() const = 0;
		
		virtual void Start(uint32_t receiverSSRC, uint32_t authorSSRC, uint32_t deviceId, std::string_view secureKey) = 0;
		virtual bool Started() const = 0;
		virtual void Stop() = 0;
		
		virtual void Pause() = 0;
		virtual void Resume() = 0;
	protected:
		~IRendererVideoSession() {}
	};
}
