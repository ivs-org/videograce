/**
 * IVideoRenderer.h - Contains interface of video renderer
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Video/Resolution.h>
#include <Proto/DeviceType.h>

#include <string>
#include <cstdint>

namespace VideoRenderer
{

class IVideoRenderer
{
public:
	virtual void SetName(const std::string &name) = 0;
	virtual void SetId(uint32_t id, int64_t clientId) = 0;

    virtual void SetDeviceType(Proto::DeviceType deviceType) = 0;
    virtual Proto::DeviceType GetDeviceType() = 0;

	virtual void Start() = 0;
	virtual void Stop() = 0;

	virtual void SetResolution(Video::Resolution resolution = Video::rVGA) = 0;
	virtual void SetMirrorVideo(bool yes) = 0;
	virtual bool GetVideoMirrored() const = 0;

	virtual void SetSpeak(bool speak) = 0;

protected:
	~IVideoRenderer() {}
};

}
