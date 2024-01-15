/**
 * ICaptureVideoSession.h - Contains interface of capture video session
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <string>

#include <Video/Resolution.h>
#include <Video/ColorSpace.h>
#include <Video/IVideoEncoder.h>

#include <Camera/ICamera.h>

#include <Proto/DeviceType.h>

namespace Transport
{
	class ISocket;
}

namespace CaptureSession
{

class ICaptureVideoSession
{
public:
	virtual void SetDeviceType(Proto::DeviceType deviceType) = 0;
	virtual void SetName(std::string_view name) = 0;
	virtual void SetResolution(Video::Resolution resolution) = 0;
	virtual void SetBitrate(int bitrate) = 0;
	virtual void SetFrameRate(uint32_t rate) = 0;

	virtual void SetEncoderType(Video::CodecType et) = 0;
	
	virtual void SetRCActions(bool yes) = 0;
	virtual bool IsRCActionsEnabled() const = 0;

	virtual void ForceKeyFrame() = 0;

	virtual void SetRTPParams(std::string_view addr, uint16_t rtpPort) = 0;
	virtual void SetWSMParams(std::string_view addr, std::string_view acessToken, std::string_view wsDestAddr) = 0;

	virtual std::string_view GetName() const = 0;
	virtual uint32_t GetDeviceId() = 0;
	virtual Proto::DeviceType GetDeviceType() = 0;

	virtual Video::Resolution GetResolution() = 0;

	virtual int GetBitrate() = 0;

	virtual void SetLocalReceiver(Transport::ISocket* receiver) = 0;

	virtual void Start(uint32_t ssrc, Video::ColorSpace colorSpace, uint32_t deviceId, std::string_view secureKey) = 0;
	virtual bool Started() const = 0;
	virtual void Stop() = 0;

	virtual void Pause() = 0;
	virtual void Resume() = 0;

	virtual void Move(Camera::MoveAxis axis, Camera::MoveType type, int value) = 0;

	virtual void Zoom(int value) = 0;
protected:
	~ICaptureVideoSession() {}
};

}
