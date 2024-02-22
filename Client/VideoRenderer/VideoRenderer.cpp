/**
 * VideoRenderer.cpp - Contains VideoRenderer's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014 - 2022
 */

#include "VideoRenderer.h"

#include <wui/graphic/graphic.hpp>

#include <atomic>

#include <Transport/RTP/RTPPacket.h>
#include <Common/WindowsVersion.h>

#include <wui/window/window.hpp>
#include <wui/system/tools.hpp>
#include <wui/theme/theme.hpp>

#include <resource.h>

namespace VideoRenderer
{

/// VideoRenderer
VideoRenderer::VideoRenderer()
	: parent_(), position_(),
    showed_(true), runned(false),
    bufferWidth(0), bufferHeight(0),
	name(),
	id(0), clientId(0),
    deviceType(Proto::DeviceType::Camera),
	resolution(Video::rVGA),
	mirror(false),
	dstSz{ 0, 0 }, prevSz{ 0, 0 },
	mirrorBuffer(), scaleBuffer(), scaleInitBuffer(), scaleSpecBuffer(), scaleWorkBuffer(),
	nowSpeak(false),
	deviceNotifyCallback(),
	err{},
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

VideoRenderer::~VideoRenderer()
{
	Stop();
}

void VideoRenderer::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    deviceNotifyCallback = deviceNotifyCallback_;
}

void VideoRenderer::draw(wui::graphic &gr, const wui::rect &)
{
	//using namespace std::chrono;
	//auto start = high_resolution_clock::now();

    if (!runned || !showed_ || bufferWidth == 0 || bufferHeight == 0)
    {
        return;
    }

    auto pos = position();

	auto color = nowSpeak ? wui::make_color(54, 183, 41) : wui::make_color(250, 250, 250);

	if (deviceType != Proto::DeviceType::Avatar)
    {
        gr.draw_buffer({ pos.left, pos.top, pos.left + bufferWidth, pos.top + bufferHeight }, scaleBuffer.get(), 0, 0);
    }
    else
    {
		gr.draw_rect(pos, wui::make_color(65, 65, 65));
#ifdef _WIN32
		gr.draw_text(pos,
			"👤",
			color,
			wui::font {
					Common::IsWindows10OrGreater() ? "Segoe UI Emoji" : "Segoe UI Symbol",
					static_cast<int32_t>(pos.height() * 0.4)
				});
#else
		gr.draw_text(pos,
			"&",
			color,
			wui::font{ "D050000L", static_cast<int32_t>(pos.height() * 0.8)	});
#endif
    }

    gr.draw_text({pos.left + 11, pos.bottom - 24}, name,
        wui::make_color(50, 50, 50),
        wui::theme_font("window", "caption_font"));

    gr.draw_text({ pos.left + 10, pos.bottom - 25 }, name,
        color,
        wui::theme_font("window", "caption_font"));

    if (nowSpeak)
    {
        gr.draw_line({ pos.left, pos.top, pos.right, pos.top }, color, 1);
        gr.draw_line({ pos.right - 1, pos.top, pos.right - 1, pos.bottom }, color, 1);
        gr.draw_line({ pos.right, pos.bottom - 1, pos.left, pos.bottom - 1 }, color, 1);
        gr.draw_line({ pos.left, pos.top, pos.left, pos.bottom }, color, 1);
    }

	/*auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	OutputDebugStringA(std::to_string(duration.count()).c_str());
	OutputDebugStringA("\n");*/
}

void VideoRenderer::set_position(const wui::rect &position__, bool redraw)
{
    update_control_position(position_, position__, redraw, parent_);
}

wui::rect VideoRenderer::position() const
{
    return get_control_position(position_, parent_);
}

void VideoRenderer::set_parent(std::shared_ptr<wui::window> window_)
{
    parent_ = window_;
}

std::weak_ptr<wui::window> VideoRenderer::parent() const
{
    return parent_;
}

void VideoRenderer::clear_parent()
{
    parent_.reset();
}

void VideoRenderer::set_topmost(bool)
{
}

bool VideoRenderer::topmost() const
{
    return false;
}

void VideoRenderer::show()
{
    showed_ = true;
}

void VideoRenderer::hide()
{
    showed_ = false;
}

bool VideoRenderer::showed() const
{
    return showed_;
}

bool VideoRenderer::enabled() const
{
    return true;
}

bool VideoRenderer::focused() const
{
    return false;
}

bool VideoRenderer::focusing() const
{
    return true;
}

wui::error VideoRenderer::get_error() const
{
	return err;
}

void VideoRenderer::SetName(std::string_view name_)
{
	name = name_;
}

void VideoRenderer::SetId(uint32_t id_, int64_t clientId_)
{
	id = id_;
	clientId = clientId_;
}

void VideoRenderer::SetDeviceType(Proto::DeviceType deviceType_)
{
    deviceType = deviceType_;
}

Proto::DeviceType VideoRenderer::GetDeviceType()
{
    return deviceType;
}

void VideoRenderer::Start()
{
	if (!runned)
	{
		CreateBuffers();
        runned = true;
        nowSpeak = false;
	}
}

void VideoRenderer::Stop()
{
	runned = false;

    DestroyBuffers();
    
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());
    }
}

void VideoRenderer::CreateBuffers()
{
	auto rv = Video::GetValues(resolution);

	try
	{
		mirrorBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[rv.width * rv.height * 4]);
		scaleBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[3840 * 2160 * 4]);
		scaleInitBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[rv.width * rv.height * 4]);
		scaleSpecBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[rv.width * rv.height * 4]);
		scaleWorkBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[3840 * 2160 * 4]);
	}
	catch (std::bad_alloc &)
	{
		if (deviceNotifyCallback)
		{
            deviceNotifyCallback(name, Client::DeviceNotifyType::MemoryError, Proto::DeviceType::VideoRenderer, id, 0);
		}
		return;
	}

	prevSz = { 0, 0 };
}

void VideoRenderer::DestroyBuffers()
{
    mirrorBuffer.reset(nullptr);
	scaleBuffer.reset(nullptr);
	scaleInitBuffer.reset(nullptr);
	scaleSpecBuffer.reset(nullptr);
	scaleWorkBuffer.reset(nullptr);
}

void VideoRenderer::SetResolution(Video::Resolution resolution_)
{
	if (runned)
	{
        runned = false;

		DestroyBuffers();
		resolution = resolution_;
		CreateBuffers();
        runned = true;
	}
	else
	{
		resolution = resolution_;
	}
}

void VideoRenderer::SetMirrorVideo(bool yes)
{
	mirror = yes;
}

bool VideoRenderer::GetVideoMirrored() const
{
	return mirror;
}

void VideoRenderer::SetSpeak(bool speak)
{
	nowSpeak = speak;
}

void VideoRenderer::ResizeImage_C4R(const Ipp8u* pSrc, IppiSize srcSize, Ipp32s srcStep, Ipp8u* pDst, IppiSize dstSize, Ipp32s dstStep)
{
	if (prevSz.width != dstSize.width || prevSz.height != dstSize.height)
	{
		auto status = ippiResizeLanczosInit_8u(srcSize, dstSize, 3, (IppiResizeSpec_32f*)scaleSpecBuffer.get(), scaleInitBuffer.get());
		if (status != ippStsNoErr)
		{
			errLog->critical("ippiResizeLanczosInit_8u error: {0:d}", status);
			return;
		}
	}

	ippiResizeLanczos_8u_C4R(pSrc, srcStep, pDst, dstStep, { 0, 0 }, dstSize, ippBorderRepl, 0, (IppiResizeSpec_32f*)scaleSpecBuffer.get(), scaleWorkBuffer.get());
}

void VideoRenderer::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	//using namespace std::chrono;
	//auto start = high_resolution_clock::now();

	if (!runned)
	{
		return;
	}
 
	auto rv = Video::GetValues(resolution);

	if (rv.width == 0 || rv.height == 0)
	{
		return;
	}

    bufferWidth = position_.width();
    bufferHeight = position_.height();

	if (bufferWidth < 10 || bufferHeight < 10)
	{
		return;
	}

	auto &packet = *static_cast<const Transport::RTPPacket*>(&packet_);

	const uint8_t *data = packet.payload;
	const IppiSize srcSz = { rv.width, rv.height };

	if (mirror)
	{
		ippiMirror_8u_C4R(data, rv.width * 4, mirrorBuffer.get(), rv.width * 4, srcSz, ippAxsVertical);
		data = mirrorBuffer.get();
	}

    auto scale = static_cast<double>(bufferWidth) / rv.width;
	
	dstSz = { bufferWidth,  static_cast<int>((static_cast<double>(rv.height) * scale)) };

    if (dstSz.width > 3840 || dstSz.height > 2160)
	{
		return;
	}

	ResizeImage_C4R(data, srcSz, rv.width * 4, scaleBuffer.get(), dstSz, dstSz.width * 4);

    prevSz = dstSz;

	/*auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	OutputDebugStringA(std::to_string(duration.count()).c_str());
	OutputDebugStringA("\n");*/
}

}
