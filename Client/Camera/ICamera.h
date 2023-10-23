/**
 * ICamera.h - Contains interface of camera
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <string>

#include <Video/Resolution.h>
#include <Video/ColorSpace.h>

#include <UI/DeviceNotifies.h>

namespace Camera
{

enum class MoveAxis
{
	Horizont,
	Vertical
};

enum class MoveType
{
	Relative,
	Absolute
};

class ICamera
{
public:
	virtual void SetName(const std::string &name) = 0;
	virtual void SetDeviceId(uint32_t id) = 0;

	virtual void Start(Video::ColorSpace colorSpace) = 0;
	virtual void Stop() = 0;

	virtual bool SetResolution(Video::Resolution resolution = Video::rVGA) = 0;
	virtual void SetFrameRate(uint32_t rate) = 0;

    virtual void SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback) = 0;
	
	virtual void Move(MoveAxis axis, MoveType type, int value) = 0;

	virtual void Zoom(int value) = 0;

	virtual ~ICamera() {};
};

}
