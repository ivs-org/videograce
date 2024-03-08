/**
 * VideoRenderer.h - Contains video renderer's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014 - 2022, 2024
 */

#pragma once

#include "IVideoRenderer.h"

#include <wui/control/i_control.hpp>
#include <wui/control/image.hpp>

#include <Transport/ISocket.h>
#include <Transport/RTP/OwnedRTPPacket.h>

#include <UI/DeviceNotifies.h>

#include <spdlog/spdlog.h>

namespace VideoRenderer
{

class VideoRenderer : public wui::i_control, public std::enable_shared_from_this<VideoRenderer>, public IVideoRenderer
{
public:
	VideoRenderer();
	~VideoRenderer();

    void SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback);

    void SetResizeCallback(std::function<void(int32_t, int32_t)>);

    /// wui::i_control impl
    virtual void draw(wui::graphic &gr, const wui::rect &);

    virtual void set_position(const wui::rect &position, bool redraw = true);
    virtual wui::rect position() const;

    virtual void set_parent(std::shared_ptr<wui::window> window_);
    virtual std::weak_ptr<wui::window> parent() const;
    virtual void clear_parent();
    
    virtual void set_topmost(bool);
    virtual bool topmost() const;

    virtual void update_theme_control_name(std::string_view theme_control_name) {}
    virtual void update_theme(std::shared_ptr<wui::i_theme> theme_ = nullptr) {}

    virtual void show();
    virtual void hide();
    virtual bool showed() const;

    virtual void enable() {}
    virtual void disable() {}
    virtual bool enabled() const;

    virtual bool focused() const;
    virtual bool focusing() const;

    virtual wui::error get_error() const;

	/// Impl of IVideoRenderer
	virtual void SetName(std::string_view name);
	virtual void SetId(uint32_t id, int64_t clientId);
    virtual void SetAudioSession(std::weak_ptr<RendererSession::IRendererAudioSession>);
    virtual void SetDeviceType(Proto::DeviceType deviceType);
    virtual Proto::DeviceType GetDeviceType();
	virtual void Start(std::function<void(Transport::OwnedRTPPacket&)> rgbSource);
	virtual void Stop();
	virtual void SetSpeak(bool speak);

private:
    std::function<void(Transport::OwnedRTPPacket&)> rgbSource;

    std::function<void(int32_t, int32_t)> resizeCallback;

    std::weak_ptr<wui::window> parent_;
    wui::rect position_;

    bool showed_, runned;

	std::string name;
	uint32_t id;
	int64_t clientId;

    std::weak_ptr<RendererSession::IRendererAudioSession> audioSession;

    Proto::DeviceType deviceType;

	bool nowSpeak;

    std::unique_ptr<uint8_t[]> flickerBuffer;

    Client::DeviceNotifyCallback deviceNotifyCallback;

    wui::error err;

	std::shared_ptr<spdlog::logger> sysLog, errLog;
};

}
