/**
 * Device.cpp - Contains device loading helpers
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <Device/Device.h>

#include <wui/locale/locale.hpp>

#ifdef _WIN32
#include <Device/DS/DSCommon.h>
#endif

namespace Client
{

#ifdef _WIN32

void LoadCameras(std::vector<Device> &devices)
{
    GetCategoryFilters(CLSID_VideoInputDeviceCategory, devices);

    for (auto &device : devices)
    {
        device.type = DeviceType::Camera;
    }

    Device device;
    device.type = DeviceType::ScreenCapturer;
    device.name = wui::locale("device", "screen_capturer");

    devices.emplace_back(device);
}

void LoadMicrophones(std::vector<Device> &devices)
{
    GetCategoryFilters(CLSID_AudioInputDeviceCategory, devices);

    for (auto &device : devices)
    {
        device.type = DeviceType::Microphone;
    }
}

#else

void LoadCameras(std::vector<Device> &devices)
{
    Device device;
    device.type = DeviceType::Camera;
    device.name = "/dev/video0";
    device.formats = { //Video::VideoFormat(Video::GetResolution(Video::ResolutionValues(320, 240)), Video::ColorSpace::YUY2),
        Video::VideoFormat(Video::GetResolution(Video::ResolutionValues(640, 480)), Video::ColorSpace::YUY2),
        Video::VideoFormat(Video::GetResolution(Video::ResolutionValues(1280, 720)), Video::ColorSpace::YUY2),
        Video::VideoFormat(Video::GetResolution(Video::ResolutionValues(1920, 1080)), Video::ColorSpace::YUY2) };
    devices.emplace_back(device);

    device.type = DeviceType::ScreenCapturer;
    device.name = wui::locale("device", "screen_capturer");

    devices.emplace_back(device);
}

void LoadMicrophones(std::vector<Device> &devices)
{
    Device device;
    device.type = DeviceType::Microphone;
    device.name = "default";
    devices.emplace_back(device);
}

#endif

Video::Resolution GetBestResolution(const std::vector<Video::VideoFormat> &formats, uint16_t height)
{
    if (formats.empty())
    {
        return Video::Resolution();
    }
    Video::Resolution result = formats.begin()->resolution;

    for (auto &format : formats)
    {
        Video::ResolutionValues rv = Video::GetValues(format.resolution);
        double relation = (double)rv.height / (double)rv.width;

        if (rv.height == height)
        {
            if (relation < 0.75)
            {
                return format.resolution;
            }
            else
            {
                result = format.resolution;
            }
        }
    }
    return result;
}

Video::ColorSpace GetBestColorSpace(const std::vector<Video::VideoFormat> &formats, Video::Resolution resolution)
{
    for (auto &format : formats)
    {
        if (format.resolution == resolution)
        {
            return *format.colorSpaces.begin();
        }
    }

    return Video::ColorSpace::Unsupported;
}

}
