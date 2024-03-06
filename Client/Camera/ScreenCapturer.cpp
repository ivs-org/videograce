/**
* ScreenCapturer.cpp - Contains the screen capturer's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2016
*/

#include <Camera/ScreenCapturer.h>

#ifdef _WIN32
#include <Camera/win/ScreenCapturerImpl.h>
#elif __linux__
#include <Camera/lin/ScreenCapturerImpl.h>
#endif

namespace Camera
{

ScreenCapturer::ScreenCapturer(Common::TimeMeter &timeMeter_, Transport::ISocket &receiver_)
    : impl(new ScreenCapturerImpl(timeMeter_, receiver_))
{
}

ScreenCapturer::~ScreenCapturer()
{
}

void ScreenCapturer::SetName(std::string_view )
{
	/// Not supported
}

void ScreenCapturer::SetDeviceId(uint32_t id)
{
    impl->SetDeviceId(id);
}

void ScreenCapturer::Move(MoveAxis axis, MoveType type, int value)
{
	/// Not supported
}

void ScreenCapturer::Zoom(int value)
{
	/// Not supported
}

void ScreenCapturer::SetRCMode(bool yes)
{
    impl->SetRCMode(yes);
}

void ScreenCapturer::Start(Video::ColorSpace cs, ssrc_t ssrc)
{
    impl->Start(cs, ssrc);
}

void ScreenCapturer::Stop()
{
    impl->Stop();
}

bool ScreenCapturer::SetResolution(Video::Resolution resolution)
{
    return impl->SetResolution(resolution);
}

void ScreenCapturer::SetFrameRate(uint32_t rate)
{
    impl->SetFrameRate(rate);
}

void ScreenCapturer::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    impl->SetDeviceNotifyCallback(deviceNotifyCallback_);
}

void ScreenCapturer::MakeMouseAction(const uint8_t *payload)
{
    impl->MakeMouseAction(payload);
}

void ScreenCapturer::MakeKeyboardAction(const uint8_t *payload)
{
    impl->MakeKeyboardAction(payload);
}

}
