/**
* ScreenCapturer.cpp - Contains the screen capturer's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2016
*/

#include <Camera/win/ScreenCapturerImpl.h>

#include <Transport/RTP/RTPPacket.h>
#include <Transport/RTP/RTCPPacket.h>

#include <Common/ShortSleep.h>

#include <wui/config/config.hpp>

#include <wui/locale/locale.hpp>

#include <ippcc.h>
#include <ippi.h>

#include <assert.h>

#ifndef _WIN32
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif

namespace Camera
{

ScreenCapturerImpl::ScreenCapturerImpl(Common::TimeMeter &timeMeter_, Transport::ISocket &receiver_)
	: receiver(receiver_),
	timeMeter(timeMeter_),
	rv(0, 0),
	left(0), top(0),
	buffer(nullptr),
	packetDuration(40000),
	deviceId(0),
	curMonNo(0),
	screenScale(100),
	captureWnd(0),
	prevSize{ 0 },
	windowCapturing(false),
	deviceNotifyCallback(),
	runned(false),
	thread(),
	rcMode(false),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

ScreenCapturerImpl::~ScreenCapturerImpl()
{
	Stop();
}

void ScreenCapturerImpl::SetName(std::string_view )
{
	/// Not supported
}

void ScreenCapturerImpl::SetDeviceId(uint32_t id)
{
	deviceId = id;
}

void ScreenCapturerImpl::Move(MoveAxis axis, MoveType type, int value)
{
	/// Not supported
}

void ScreenCapturerImpl::Zoom(int value)
{
	/// Not supported
}

void ScreenCapturerImpl::SetRCMode(bool yes)
{
	rcMode = yes;
}

void ScreenCapturerImpl::Start(Video::ColorSpace)
{
	if (!runned)
	{
		try
		{
			buffer = std::unique_ptr<uint8_t[]>(new uint8_t[4096 * 3072 * 4]);
		}
		catch (std::bad_alloc &)
		{
			if (deviceNotifyCallback)
			{
                deviceNotifyCallback(wui::locale("device", "screen_capturer"), Client::DeviceNotifyType::MemoryError, Proto::DeviceType::Demonstration, deviceId, 0);
			}
			return;
		}

		captureWnd = (HWND)wui::config::get_int64("CaptureDevices\\" + wui::locale("device", "screen_capturer"), "CaptureWnd", (int64_t)GetDesktopWindow());
		screenScale = wui::config::get_int("CaptureDevices\\" + wui::locale("device", "screen_capturer"), "ScreenScale", 100);

		windowCapturing = captureWnd != GetDesktopWindow();

		if (!windowCapturing)
		{
			left = wui::config::get_int("CaptureDevices\\" + wui::locale("device", "screen_capturer"), "ScreenLeft", 0);
			top = wui::config::get_int("CaptureDevices\\" + wui::locale("device", "screen_capturer"), "ScreenTop", 0);
		}

		runned = true;
		thread = std::thread(&ScreenCapturerImpl::run, this);
	}
}

void ScreenCapturerImpl::Stop()
{
	runned = false;
	if (thread.joinable()) thread.join();

	buffer.reset(nullptr);
}

bool ScreenCapturerImpl::SetResolution(Video::Resolution resolution)
{
	rv = Video::GetValues(resolution);
	return true;
}

void ScreenCapturerImpl::SetFrameRate(uint32_t rate)
{
	packetDuration = (1000 / rate) * 1000;
}

void ScreenCapturerImpl::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    deviceNotifyCallback = deviceNotifyCallback_;
}

void ScreenCapturerImpl::run()
{
	using namespace std::chrono;

	sysLog->info("ScreenCapturer started");
		
	while (runned)
	{
		auto start = high_resolution_clock::now();

		CaptureTheScreen();

		int64_t playDuration = duration_cast<microseconds>(high_resolution_clock::now() - start).count();

		if (packetDuration > playDuration) Common::ShortSleep(packetDuration - playDuration - 500);
	}

	sysLog->info("ScreenCapturer ended");
}

void ScreenCapturerImpl::CaptureTheScreen()
{
	HDC hdcMemDC = NULL;
	HBITMAP hbmScreen = NULL;
	BITMAP bmpScreen;

	CURSORINFO cursorInfo = { 0 };
	ICONINFO ii = { 0 };

	if (!IsWindow(captureWnd))
	{
		if (deviceNotifyCallback)
		{
            deviceNotifyCallback(wui::locale("device", "screen_capturer"), Client::DeviceNotifyType::DeviceEnded, Proto::DeviceType::Demonstration, deviceId, 0);
		}
		runned = false;
		return;
	}

	RECT rcWnd = { 0 };

	if (windowCapturing)
	{
		GetWindowRect(captureWnd, &rcWnd);

		if (rcWnd.right % 2 != 0 && rcWnd.left % 2 == 0)
		{
			rcWnd.right -= 1;
		}
		if (rcWnd.right % 2 == 0 && rcWnd.left % 2 != 0)
		{
			rcWnd.left += 1;
		}

		if (prevSize.x != rcWnd.right - rcWnd.left || prevSize.y != rcWnd.bottom - rcWnd.top)
		{
			rv.width = static_cast<uint16_t>(std::round((rcWnd.right - rcWnd.left) / ((double)screenScale / 100)));
			rv.height = static_cast<uint16_t>(std::round((rcWnd.bottom - rcWnd.top) / ((double)screenScale / 100)));

			prevSize = { rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top };

			if (deviceNotifyCallback)
			{
                deviceNotifyCallback(wui::locale("device", "screen_capturer"), Client::DeviceNotifyType::ResolutionChanged, Proto::DeviceType::Demonstration, deviceId, Video::GetResolution(rv));
			}
		}
	}

	Transport::RTPPacket packet;

	HDC hdcScreen = GetWindowDC(captureWnd);

	// Create a compatible DC which is used in a BitBlt from the screen DC
	hdcMemDC = CreateCompatibleDC(hdcScreen);

	if (!hdcMemDC)
	{
		errLog->critical("ScreenCapturer CreateCompatibleDC has failed");
		goto done;
	}

	// Create a compatible bitmap from the Window DC
	hbmScreen = CreateCompatibleBitmap(hdcScreen, rv.width, rv.height);

	if (!hbmScreen)
	{
		errLog->critical("ScreenCapturer CreateCompatibleBitmap Failed");
		goto done;
	}

	// Select the compatible bitmap into the compatible memory DC.
	SelectObject(hdcMemDC, hbmScreen);

	// Bit block transfer into our compatible memory DC.
	if (!BitBlt(hdcMemDC,
		0, 0,
		rv.width, rv.height,
		hdcScreen,
		left, top,
		SRCCOPY))
	{
		errLog->critical("ScreenCapturer BitBlt has failed");
		goto done;
	}

	// Capture the cursor
	if (!rcMode)
	{
		cursorInfo.cbSize = sizeof(cursorInfo);
		if (GetCursorInfo(&cursorInfo) && GetIconInfo(cursorInfo.hCursor, &ii))
		{
			int32_t cursorX = 0;
			int32_t cursorY = 0;

			if (windowCapturing)
			{
				cursorX = static_cast<int32_t>(std::round((double)(cursorInfo.ptScreenPos.x - rcWnd.left) / ((double)screenScale / 100)));
				cursorY = static_cast<int32_t>(std::round((double)(cursorInfo.ptScreenPos.y - rcWnd.top) / ((double)screenScale / 100)));
			}
			else
			{
				cursorX = static_cast<int32_t>(std::round((double)(cursorInfo.ptScreenPos.x) / ((double)screenScale / 100)));
				cursorY = static_cast<int32_t>(std::round((double)(cursorInfo.ptScreenPos.y) / ((double)screenScale / 100)));
			}

			if (cursorX >= left && cursorY >= top)
			{
				cursorX -= left;
				cursorY -= top;

				DIBSECTION dsColor = { 0 };
				DIBSECTION dsMask = { 0 };
				if (GetObject(ii.hbmColor, sizeof(DIBSECTION), &dsColor))
				{
					if (GetObject(ii.hbmMask, sizeof(DIBSECTION), &dsMask))
					{
						HDC hBDC = CreateCompatibleDC(hdcMemDC);

						SelectObject(hBDC, ii.hbmMask);
						BitBlt(hdcMemDC, cursorX - ii.xHotspot, cursorY - ii.yHotspot, dsMask.dsBm.bmWidth, dsMask.dsBm.bmHeight, hBDC, 0, 0, SRCAND);
						DeleteDC(hBDC);
					}

					HDC hBDC = CreateCompatibleDC(hdcMemDC);
					SelectObject(hBDC, ii.hbmColor);

					BitBlt(hdcMemDC, cursorX - ii.xHotspot, cursorY - ii.yHotspot, dsColor.dsBm.bmWidth, dsColor.dsBm.bmHeight, hBDC, 0, 0, SRCPAINT);

					DeleteDC(hBDC);
				}
				else if (GetObject(ii.hbmMask, sizeof(DIBSECTION), &dsMask))
				{
					HDC hBDC = CreateCompatibleDC(hdcMemDC);

					SelectObject(hBDC, ii.hbmMask);
					BitBlt(hdcMemDC, cursorX - ii.xHotspot, cursorY - ii.yHotspot, dsMask.dsBm.bmWidth, dsMask.dsBm.bmHeight / 2, hBDC, 0, 0, SRCAND);
					BitBlt(hdcMemDC, cursorX - ii.xHotspot, cursorY - ii.yHotspot, dsMask.dsBm.bmWidth, dsMask.dsBm.bmHeight / 2, hBDC, 0, dsMask.dsBm.bmHeight / 2, SRCPAINT);
					DeleteDC(hBDC);
				}

				DeleteObject(ii.hbmColor);
				DeleteObject(ii.hbmMask);
			}
		}
	}

	// Get the BITMAP from the HBITMAP
	GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

	BITMAPINFOHEADER bi;

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmpScreen.bmWidth;
	bi.biHeight = -bmpScreen.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	int bmpSize = bmpScreen.bmWidth * bmpScreen.bmHeight * 4;

	// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
	// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
	// have greater overhead than HeapAlloc.
	HANDLE hDIB = GlobalAlloc(GHND, bmpSize);
	uint8_t *lpbitmap = (uint8_t*)GlobalLock(hDIB);

	// Gets the "bits" from the bitmap and copies them into a buffer 
	// which is pointed to by lpbitmap.
	GetDIBits(hdcScreen, hbmScreen, 0,
		(UINT)bmpScreen.bmHeight,
		lpbitmap,
		(BITMAPINFO *)&bi, DIB_RGB_COLORS);

	// Sending the packet to receiver
	packet.rtpHeader.ts = (uint32_t)(timeMeter.Measure() / 1000);
	ConvertFromRGB32(&lpbitmap, &bmpSize);
	packet.payload = lpbitmap;
	packet.payloadSize = bmpSize;

	receiver.Send(packet);

	//Unlock and Free the DIB from the heap
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	//Clean up
done:
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);
	ReleaseDC(NULL, hdcScreen);
}

void ScreenCapturerImpl::ConvertFromRGB32(unsigned char** data_, int *len_)
{
	const IppiSize  sz = { rv.width, rv.height };
	Ipp8u*          dst[3] = { buffer.get(), buffer.get() + (rv.width * rv.height), buffer.get() + (rv.width * rv.height) + ((rv.width * rv.height) / 4) };
	int             dstStep[3] = { rv.width, rv.width / 2, rv.width / 2 };

	ippiBGRToYCbCr420_8u_AC4P3R(*data_, rv.width * 4, dst, dstStep, sz);

	*data_ = buffer.get();
	*len_ = (rv.width * rv.height) + ((rv.width * rv.height) / 2);
}

void ScreenCapturerImpl::MakeMouseAction(int x, int y, int flags)
{
	RECT rcWnd = { 0 };
	if (windowCapturing)
	{
		GetWindowRect(captureWnd, &rcWnd);
	}

	SetCursorPos((int)round((double)(x) * (double)screenScale / 100) + rcWnd.left, (int)round((double)(y) * (double)screenScale / 100) + rcWnd.top);

	INPUT input;

	ZeroMemory(&input, sizeof(input));

	input.type = INPUT_MOUSE;
	input.mi.mouseData = 0;
	input.mi.dwFlags = flags;
	SendInput(1, &input, sizeof(input));
}

void ScreenCapturerImpl::MakeMouseAction(const uint8_t *payload)
{
	Transport::RTCPPacket::RemoteControlAction action = static_cast<Transport::RTCPPacket::RemoteControlAction>(ntohs(*reinterpret_cast<const uint16_t*>(payload)));
	int x = ntohs(*reinterpret_cast<const uint16_t*>(payload + 2)) + left;
	int y = ntohs(*reinterpret_cast<const uint16_t*>(payload + 4)) + top;

	int flags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;

	switch (action)
	{
	case Transport::RTCPPacket::rcaMove:
		break;
	case Transport::RTCPPacket::rcaLeftUp:
		flags |= MOUSEEVENTF_LEFTUP;
		break;
	case Transport::RTCPPacket::rcaLeftDown:
		flags |= MOUSEEVENTF_LEFTDOWN;
		break;
	case Transport::RTCPPacket::rcaCenterUp:
		flags |= MOUSEEVENTF_MIDDLEUP;
		break;
	case Transport::RTCPPacket::rcaCenterDown:
		flags |= MOUSEEVENTF_MIDDLEDOWN;
		break;
	case Transport::RTCPPacket::rcaRightUp:
		flags |= MOUSEEVENTF_RIGHTUP;
		break;
	case Transport::RTCPPacket::rcaRightDown:
		flags |= MOUSEEVENTF_RIGHTDOWN;
		break;
	case Transport::RTCPPacket::rcaLeftDblClick:
		flags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTDOWN;
		MakeMouseAction(x, y, flags);
		flags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTUP;
		MakeMouseAction(x, y, flags);

		flags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTDOWN;
		MakeMouseAction(x, y, flags);
		flags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTUP;
		MakeMouseAction(x, y, flags);
		break;
	case Transport::RTCPPacket::rcaRightDblClick:
		flags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_RIGHTDOWN;
		MakeMouseAction(x, y, flags);
		flags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_RIGHTUP;
		MakeMouseAction(x, y, flags);

		flags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_RIGHTDOWN;
		MakeMouseAction(x, y, flags);
		flags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_RIGHTUP;
		MakeMouseAction(x, y, flags);
		break;
	case Transport::RTCPPacket::rcaWheel:
	{
		INPUT input;
		ZeroMemory(&input, sizeof(input));
		input.type = INPUT_MOUSE;
		input.mi.mouseData = ntohl(*reinterpret_cast<const int32_t*>(payload + 4));
		input.mi.dwFlags = MOUSEEVENTF_WHEEL;
		SendInput(1, &input, sizeof(input));
	}
	break;
	}

	if (action != Transport::RTCPPacket::rcaWheel && action != Transport::RTCPPacket::rcaLeftDblClick && action != Transport::RTCPPacket::rcaRightDblClick)
	{
		MakeMouseAction(x, y, flags);
	}
}

void ScreenCapturerImpl::MakeKeyboardAction(const uint8_t *payload)
{
	bool keyDown = ntohs(*reinterpret_cast<const uint16_t*>(payload)) == Transport::RTCPPacket::rcaKeyDown;
	int16_t modifier = ntohs(*reinterpret_cast<const uint16_t*>(payload + 2));
	int32_t key = ntohl(*reinterpret_cast<const int32_t*>(payload + 4));

	INPUT input = { 0 };
	ZeroMemory(&input, sizeof(input));
	input.type = INPUT_KEYBOARD;
	input.ki.dwFlags = key != VK_RETURN ? (keyDown ? 0 : KEYEVENTF_KEYUP) : KEYEVENTF_UNICODE;
	input.ki.time = 0;
	input.ki.dwExtraInfo = static_cast<ULONG_PTR>(0);

	input.ki.wVk = key != VK_RETURN ? key : 0;
	input.ki.wScan = key != VK_RETURN ? 0 : VK_RETURN;

	SendInput(1, &input, sizeof(input));
}

}
