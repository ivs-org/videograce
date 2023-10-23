/**
* ScreenCapturer.cpp - Contains the screen capturer's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2016
*/

#include <Camera/lin/ScreenCapturerImpl.h>
#include <Transport/RTP/RTPPacket.h>

#include <ippcc.h>
#include <ippi.h>

#include <assert.h>

#include <X11/Xutil.h>
#include <X11/Xmd.h>
#include <X11/Xatom.h>

namespace Camera
{

ScreenCapturerImpl::ScreenCapturerImpl(Common::TimeMeter &timeMeter_, Transport::ISocket &receiver_)
    : timeMeter(timeMeter_), receiver(receiver_),
	frameRate(20),
	buffer0(new uint8_t[3840 * 2160 * 3]),
	buffer1(new uint8_t[3840 * 2160 * 3]),
	buffer2(new uint8_t[3840 * 2160 * 3]),
	dis(nullptr), scr(nullptr),
	runned(false),
	thread(),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

ScreenCapturerImpl::~ScreenCapturerImpl()
{
	Stop();
}

void ScreenCapturerImpl::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
}

void ScreenCapturerImpl::Move(MoveAxis axis, MoveType type, int value)
{
}

void ScreenCapturerImpl::Zoom(int value)
{
}

void ScreenCapturerImpl::SetRCMode(bool yes)
{

}

void ScreenCapturerImpl::MakeKeyboardAction(const uint8_t *payload)
{

}

void ScreenCapturerImpl::MakeMouseAction(const uint8_t *payload)
{

}

void ScreenCapturerImpl::SetName(const std::string &name_)
{
	/// Not supported
}

void ScreenCapturerImpl::SetDeviceId(uint32_t id)
{

}

void ScreenCapturerImpl::Start(Video::ColorSpace)
{
	if (!runned)
	{
		runned = true;
		thread = std::thread([this]() {
			sysLog->info("ScreenCapturer started");

			dis = XOpenDisplay(NULL);
			scr = XDefaultScreenOfDisplay(dis);

			while (runned)
			{
				CaptureTheScreen();
				usleep(1000000 / frameRate);
			}

			XCloseDisplay(dis);

			sysLog->info("ScreenCapturer ended");
		});
	}
}

void ScreenCapturerImpl::Stop()
{
	if (runned)
	{
		runned = false;
		thread.join();
	}
}

bool ScreenCapturerImpl::SetResolution(Video::Resolution)
{
	/// Not supported
	return false;
}

void ScreenCapturerImpl::SetFrameRate(uint32_t rate)
{
	frameRate = rate;
}

void ScreenCapturerImpl::CaptureTheScreen()
{
	Drawable drawable = XDefaultRootWindow(dis);

	XImage *image = XGetImage(dis, drawable, 0, 0, scr->width, scr->height, AllPlanes, ZPixmap);

	int outPos = 0;
	for (int y = scr->height; y != 0; --y)
	{
		for (int x = scr->width; x != 0; --x)
		{
			int inPos = ((y * scr->width) - x) * 4 - 4;

			buffer0.get()[outPos] = image->data[inPos];
			buffer0.get()[outPos + 1] = image->data[inPos + 1];
			buffer0.get()[outPos + 2] = image->data[inPos + 2];

			outPos += 3;
		}
	}

	XDestroyImage(image);

	int size = 0;

	// Sending the packet to receiver
	Transport::RTPPacket packet;
	packet.rtpHeader.ts = timeMeter.Measure();
	ConvertFromRGB24(buffer0.get(), &size, scr->width, scr->height);
	packet.payload = buffer2.get();
	packet.payloadSize = size;

	receiver.Send(packet);
}

void ScreenCapturerImpl::ConvertFromRGB24(unsigned char* data_, int *len_, int width, int height)
{
	const IppiSize  sz = { width, height };
	Ipp8u*          dst[3] = { buffer2.get(), buffer2.get() + (width * height), buffer2.get() + (width * height) + ((width * height)/4) };
	int             dstStep[3] = { width, width / 2, width / 2 };

	ippiMirror_8u_C3R(data_, width * 3, buffer1.get(), width * 3, sz, ippAxsHorizontal);
	ippiBGRToYCbCr420_8u_C3P3R(buffer1.get(), width * 3, dst, dstStep, sz);

	*len_ = (width * height) + ((width * height) / 2);
}

Video::Resolution GetScreenResolution()
{
	Display *dis = XOpenDisplay((char *)0);
	Screen *scr = XDefaultScreenOfDisplay(dis);

	auto res = Video::GetResolution(Video::ResolutionValues(scr->width, scr->height));

	XCloseDisplay(dis);

	return res;
}

}
