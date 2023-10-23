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
	virtual void SetName(const std::string &name) final;
	virtual void SetDeviceId(uint32_t id) final;
	virtual void Start(Video::ColorSpace colorSpace) final;
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

	uint64_t packetDuration;

	uint32_t dataLength;

	uint64_t processTime;
	uint32_t overTimeCount;
	
	StreamConfigPtr streamConfig;
	KsPropertySetPtr ksPropertySet;
	AMCameraControlPtr cameraControl;

	CComQIPtr<IMediaControl, &IID_IMediaControl> mediaControl;

	std::string name;
	uint32_t deviceId;
	Video::Resolution resolution;
	Video::ColorSpace colorSpace;

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
