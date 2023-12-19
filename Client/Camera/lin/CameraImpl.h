/**
 * CameraImpl.h - Contains camera's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2015
 */

#pragma once

#include <Camera/ICamera.h>
#include <Transport/ISocket.h>
#include <Common/TimeMeter.h>

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
	~CameraImpl();

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

private:
    Common::TimeMeter &timeMeter;
    Transport::ISocket &receiver;

	std::unique_ptr<uint8_t[]> buffer, tmpBuffer;

    uint64_t packetDuration;
	
    Client::DeviceNotifyCallback deviceNotifyCallback;

	std::string name;
	uint32_t deviceId;
	Video::Resolution resolution;

	std::atomic<bool> runned;
	std::thread thread;

	uint32_t processTime;
	uint32_t overTimeCount;

	int fd;

	struct Buffer
	{
		void   *data;
		size_t  size;
	};

	Buffer *buffers;
	unsigned int n_buffers;

	bool flipHorizontal;

	std::shared_ptr<spdlog::logger> sysLog, errLog;

	/// methods

	int init_mmap();

	int open_device();
	void close_device();

	int init_device();
	void uninit_device();

	int start_capturing();
	void stop_capturing();

	bool read_frame();

	void Postprocess(const uint8_t* data, int len);
	void FlipHorizontal(const uint8_t* data_);

	void run();
};

}
