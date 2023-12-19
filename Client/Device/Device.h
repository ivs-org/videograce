/**
 * Device.h - Contains device parameters struct
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2022
 */

#pragma once

#include <Video/VideoFormat.h>

#include <string>

namespace Client
{

enum class DeviceType
{
    Undefined,
    Camera,
    ScreenCapturer,
    Microphone
};

struct Device
{
    DeviceType type;
	std::string name;

    std::vector<Video::VideoFormat> formats;

    Device()
		: type(DeviceType::Undefined), name(), formats()
	{
	}

    inline bool operator==(std::string_view name_)
	{
		return name == name_;
	}
};

void LoadCameras(std::vector<Device> &devices);
void LoadMicrophones(std::vector<Device> &devices);

Video::Resolution GetBestResolution(const std::vector<Video::VideoFormat> &formats, uint16_t height);
Video::ColorSpace GetBestColorSpace(const std::vector<Video::VideoFormat> &formats, Video::Resolution resolution);

}
