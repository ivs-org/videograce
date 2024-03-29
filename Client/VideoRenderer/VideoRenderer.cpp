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

#include <RendererSession/IRendererAudioSession.h>
#include <JitterBuffer/JB.h>

#include <resource.h>

namespace VideoRenderer
{

/// VideoRenderer
VideoRenderer::VideoRenderer()
	: rgbSource(),
	resizeCallback(),
    slowRenderingCallback(),
	parent_(), position_(),
    showed_(true), runned(false),
	name(),
	id(0), clientId(0),
    statMeter(50),
    audioSessionMutex(),
    audioSession(),
    deviceType(Proto::DeviceType::Camera),
	nowSpeak(false),
    flickerBuffer(),
	err{},
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

VideoRenderer::~VideoRenderer()
{
	Stop();
}

void VideoRenderer::SetResizeCallback(std::function<void(int32_t, int32_t)> resizeCallback_)
{
	resizeCallback = resizeCallback_;
}


void VideoRenderer::SetSlowRenderingCallback(std::function<void(int64_t)> callback)
{
    slowRenderingCallback = callback;
}

void VideoRenderer::draw(wui::graphic &gr, const wui::rect &)
{
    if (!runned || !showed_)
    {
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();
	
	auto pos = position();

	auto color = nowSpeak ? wui::make_color(54, 183, 41) : wui::make_color(250, 250, 250);

	if (deviceType != Proto::DeviceType::Avatar)
    {
		Transport::OwnedRTPPacket packet;
		rgbSource(packet);

        int32_t size = pos.width() * pos.height() * 4;
        if (packet.size == size)
        {
            memcpy(flickerBuffer.get(), packet.data, size);
        }

        gr.draw_buffer(pos,
            flickerBuffer.get(),
            0, 0);
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
					static_cast<int32_t>(pos.height() * 0.3)
				});
#else
		gr.draw_text(pos,
			"&",
			color,
			wui::font{ "D050000L", static_cast<int32_t>(pos.height() * 0.8)	});
#endif
        {
            std::lock_guard<std::mutex> lock(audioSessionMutex);
            auto ras = audioSession.lock();
            if (ras)
            {
                Transport::OwnedRTPPacket ortp;
                ras->GetJB().ReadFrame(ortp);
                Transport::RTPPacket rtp;
                rtp.payload = ortp.data;
                rtp.payloadSize = ortp.size;
                soundIndicator.Send(rtp, nullptr);
                soundIndicator.draw(gr, {});
            }
        }
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
        gr.draw_line({ pos.right, pos.top, pos.right, pos.bottom }, color, 1);
        gr.draw_line({ pos.right, pos.bottom, pos.left, pos.bottom }, color, 1);
        gr.draw_line({ pos.left, pos.top, pos.left, pos.bottom }, color, 1);
    }

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
    if (duration == 0)
    {
        return;
    }
    statMeter.PushVal(duration);
    auto avg = statMeter.GetAvg();

    if (statMeter.GetFill() == 40 && avg > 10) /// 10 ms deadline
    {
        statMeter.Clear();
        errLog->warn("VideoRenderer[{0}] :: Too slow rendering ({1} ms)", name, avg);
        slowRenderingCallback(duration);
    }
}

void VideoRenderer::set_position(const wui::rect &position__, bool redraw)
{
    update_control_position(position_, position__, redraw, parent_);
    
    soundIndicator.set_position({ position_.left,
        position_.bottom - (position_.height() / 5) - 30,
        position_.right,
        position_.bottom - 30 });
    
    resizeCallback(position__.width(), position__.height());
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

void VideoRenderer::SetAudioSession(std::weak_ptr<RendererSession::IRendererAudioSession> audioSession_)
{
    std::lock_guard<std::mutex> lock(audioSessionMutex);
    
    audioSession = audioSession_;
    if (audioSession.lock())
    {
        sysLog->trace("VideoRenderer :: SetAudioSession :: device_id: {0}, client_id: {1} set the audio renderer session, device_id: {2}, client_id: {3}",
            id, clientId, audioSession.lock()->GetDeviceId(), audioSession.lock()->GetClientId());
    }
    else
    {
        sysLog->trace("VideoRenderer :: SetAudioSession :: device_id: {0}, client_id: {1} remove the audio renderer session", id, clientId);
    }
}

void VideoRenderer::SetDeviceType(Proto::DeviceType deviceType_)
{
    deviceType = deviceType_;
}

Proto::DeviceType VideoRenderer::GetDeviceType()
{
    return deviceType;
}

void VideoRenderer::Start(std::function<void(Transport::OwnedRTPPacket&)> rgbSource_)
{
	if (!runned)
	{
		rgbSource = rgbSource_;

        runned = true;
        nowSpeak = false;

        flickerBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[3840 * 2160 * 4]);
	}
}

void VideoRenderer::Stop()
{
	runned = false;
    
    flickerBuffer.reset(nullptr);

    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());
    }
}

void VideoRenderer::SetSpeak(bool speak)
{
	nowSpeak = speak;
}

}
