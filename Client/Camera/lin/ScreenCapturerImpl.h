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

#include <thread>
#include <atomic>
#include <string>
#include <memory>

#include <spdlog/spdlog.h>

#include <X11/Xlib.h>

namespace Camera
{

class ScreenCapturerImpl : public ICamera
{
public:
    ScreenCapturerImpl(Common::TimeMeter &timeMeter, Transport::ISocket &receiver);
	virtual ~ScreenCapturerImpl();

	/// Impl of ICamera
    virtual void SetName(std::string_view name);
    virtual void SetDeviceId(uint32_t id);
    virtual void Start(Video::ColorSpace colorSpace);
    virtual void Stop();
    virtual bool SetResolution(Video::Resolution resolution = Video::rVGA);
    virtual void SetFrameRate(uint32_t rate);
    virtual void SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback);
    virtual void Move(MoveAxis axis, MoveType type, int value);
    virtual void Zoom(int value);

    void SetRCMode(bool yes);

    void MakeKeyboardAction(const uint8_t *payload);
    void MakeMouseAction(const uint8_t *payload);

private:
    Common::TimeMeter &timeMeter;
    Transport::ISocket &receiver;

    uint32_t frameRate;
	std::unique_ptr<uint8_t> buffer0, buffer1, buffer2;

	Display *dis;
	Screen *scr;
		
	std::atomic<bool> runned;
	std::thread thread;

	std::shared_ptr<spdlog::logger> sysLog, errLog;

	void CaptureTheScreen();
	void ConvertFromRGB24(unsigned char* data, int *len, int w, int h);
};

Video::Resolution GetScreenResolution();

}
