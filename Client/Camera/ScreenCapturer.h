/**
 * ScreenCapturer.h - Contains the screen capturer's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <Camera/ICamera.h>
#include <Transport/ISocket.h>
#include <Common/TimeMeter.h>

#include <memory>

namespace Camera
{

class ScreenCapturerImpl;

class ScreenCapturer : public ICamera
{
public:
	ScreenCapturer(Common::TimeMeter &timeMeter, Transport::ISocket &receiver);
	virtual ~ScreenCapturer();

	/// Impl of ICamera
	virtual void SetName(std::string_view ) final;
	virtual void SetDeviceId(uint32_t id) final;
	virtual void Start(Video::ColorSpace colorSpace, ssrc_t ssrc) final;
	virtual void Stop() final;
	virtual bool SetResolution(Video::Resolution resolution = Video::rVGA) final;
	virtual void SetFrameRate(uint32_t rate) final;
	
    virtual void SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback) final;

	virtual void Move(MoveAxis axis, MoveType type, int value) final;

	virtual void Zoom(int value) final;

	void SetRCMode(bool yes);

	void MakeKeyboardAction(const uint8_t *payload);
	void MakeMouseAction(const uint8_t *payload);

private:
    std::unique_ptr<ScreenCapturerImpl> impl;
};

}
