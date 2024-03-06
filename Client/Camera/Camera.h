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

#include <memory>

namespace Camera
{

class CameraImpl;

class Camera : public ICamera
{
public:
    Camera(Common::TimeMeter &timeMeter, Transport::ISocket &receiver);
    virtual ~Camera();

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
    std::unique_ptr<CameraImpl> impl;
};

}
