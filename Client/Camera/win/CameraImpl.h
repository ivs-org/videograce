/**
 * Camera.h - Contains camera's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Camera/ICamera.h>
#include <Transport/ISocket.h>
#include <Common/TimeMeter.h>
#include <Common/StatMeter.h>

#include <Camera/win/CameraDSG.h>

#include <memory>
#include <thread>
#include <atomic>
#include <string>

#include <spdlog/spdlog.h>

namespace Camera
{

class CameraImpl : public ICamera
{
public:
    CameraImpl(Common::TimeMeter &timeMeter, Transport::ISocket &receiver);
	virtual ~CameraImpl();

	/// Impl of ICamera
	virtual void SetName(std::string_view name) final;
	virtual void SetDeviceId(uint32_t id) final;
	virtual void Start(Video::ColorSpace colorSpace, ssrc_t ssrc) final;
	virtual void Stop() final;
	virtual bool SetResolution(Video::Resolution resolution = Video::rVGA) final;
	virtual void SetFrameRate(uint32_t rate) final;

    virtual void SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback) final;

    virtual void Move(MoveAxis axis, MoveType type, int value) final;

	virtual void Zoom(int value) final;

private:
    Common::TimeMeter &timeMeter;
	Transport::ISocket &receiver;
    Client::DeviceNotifyCallback deviceNotifyCallback;

	std::mutex bufferMutex;

	std::unique_ptr<uint8_t[]> captureBuffer, outputBuffer, tmpBuffer;

	uint64_t frameDuration;

	uint32_t dataLength;

	StreamConfigPtr streamConfig;
	KsPropertySetPtr ksPropertySet;
	AMCameraControlPtr cameraControl;

	CComQIPtr<IMediaControl, &IID_IMediaControl> mediaControl;

	Common::StatMeter statMeter;

	std::string name;
	uint32_t deviceId;
	Video::Resolution resolution;
	Video::ColorSpace colorSpace;

	ssrc_t ssrc;
	uint32_t seq;

	std::atomic<bool> runned;
	std::atomic<bool> restarting;

	std::thread captureThread, sendThread;
	
	std::shared_ptr<spdlog::logger> sysLog, errLog;

	HRESULT SetCameraParams(const Video::ResolutionValues &rv);

	static void CALLBACK OutputCallback(uint8_t* data, int len, void *instance);

	static void ConvertFromYUY2(unsigned char* data, int *len, CameraImpl &instance);
	static void ConvertFromUYVU(unsigned char* data, int *len, CameraImpl &instance);
	static void ConvertFromRGB24(unsigned char* data, int *len, CameraImpl &instance);
	static void ConvertFromRGB32(unsigned char* data, int *len, CameraImpl &instance);

	int GetCurrentZoom();
	
	HRESULT Capture();
	void send();
};

}
