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
#include <thread>
#include <atomic>
#include <string>

#include <spdlog/spdlog.h>

namespace Camera
{

class ScreenCapturerImpl : public ICamera
{
public:
    ScreenCapturerImpl(Common::TimeMeter &timeMeter, Transport::ISocket &receiver);
	virtual ~ScreenCapturerImpl();

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
	Transport::ISocket &receiver;
	Common::TimeMeter &timeMeter;
	Video::ResolutionValues rv;
	int16_t left, top;
	std::unique_ptr<uint8_t[]> buffer;

	int64_t packetDuration;

	uint32_t deviceId;

	ssrc_t ssrc;
	uint32_t seq;
	
	int curMonNo;

	int screenScale;
	HWND captureWnd;
	POINT prevSize;

	bool windowCapturing;

    Client::DeviceNotifyCallback deviceNotifyCallback;
		
	std::atomic<bool> runned;

	std::thread thread;

	bool rcMode;

	std::shared_ptr<spdlog::logger> sysLog, errLog;

	void CaptureTheScreen();
	
	void ConvertFromRGB32(unsigned char** data, int *len);

	void run();

	void MakeMouseAction(int x, int y, int flags);
};

}
