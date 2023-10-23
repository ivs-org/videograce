/**
* Camera.cpp - Contains camera's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2014
*/

#include <Camera/Camera.h>

#ifdef _WIN32
#include <Camera/win/CameraImpl.h>
#elif __linux__
#include <Camera/lin/CameraImpl.h>
#endif

namespace Camera
{

Camera::Camera(Common::TimeMeter &timeMeter_, Transport::ISocket &receiver_)
    : impl(new CameraImpl(timeMeter_, receiver_))
{
}

Camera::~Camera()
{
}

void Camera::Move(MoveAxis axis, MoveType type, int value)
{
    impl->Move(axis, type, value);
}

void Camera::Zoom(int value)
{
    impl->Zoom(value);
}

void Camera::SetName(const std::string &name_)
{
    impl->SetName(name_);
}

void Camera::SetDeviceId(uint32_t id)
{
    impl->SetDeviceId(id);
}

void Camera::Start(Video::ColorSpace colorSpace_)
{
    impl->Start(colorSpace_);
}

void Camera::Stop()
{
    impl->Stop();
}

bool Camera::SetResolution(Video::Resolution resolution_)
{
    return impl->SetResolution(resolution_);
}

void Camera::SetFrameRate(uint32_t rate)
{
    impl->SetFrameRate(rate);
}

void Camera::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    impl->SetDeviceNotifyCallback(deviceNotifyCallback_);
}

}
