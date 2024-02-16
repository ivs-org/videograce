/**
 * VideoRenderer.h - Contains video renderer's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014 - 2022
 */

#pragma once

#include <wui/control/i_control.hpp>
#include <wui/control/image.hpp>

#include <memory>

#include <Transport/ISocket.h>

#include <UI/DeviceNotifies.h>

#include <spdlog/spdlog.h>

#include <ippcc.h>
#include <ippi.h>

#include "IVideoRenderer.h"

namespace VideoRenderer
{

class VideoRenderer : public wui::i_control, public std::enable_shared_from_this<VideoRenderer>, public IVideoRenderer, public Transport::ISocket
{
public:
	VideoRenderer();
	~VideoRenderer();

    void SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback);

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
    virtual void SetDeviceType(Proto::DeviceType deviceType);
    virtual Proto::DeviceType GetDeviceType();
	virtual void Start();
	virtual void Stop();
	virtual void SetResolution(Video::Resolution resolution = Video::rVGA);
	virtual void SetMirrorVideo(bool yes);
	virtual bool GetVideoMirrored() const;
	virtual void SetSpeak(bool speak);

	/// Impl of Transport::ISocket
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

private:
    std::weak_ptr<wui::window> parent_;
    wui::rect position_;
    
	bool showed_, runned;

    std::mutex mutex;
    int32_t bufferWidth, bufferHeight;

	std::string name;
	uint32_t id;
	int64_t clientId;

    Proto::DeviceType deviceType;

	Video::Resolution resolution;
	bool mirror;
	
	IppiSize dstSz, prevSz;
	std::unique_ptr<uint8_t[]> mirrorBuffer,
		scaleBuffer, scaleInitBuffer, scaleSpecBuffer, scaleWorkBuffer;

	bool nowSpeak;

    Client::DeviceNotifyCallback deviceNotifyCallback;

    wui::error err;

	std::shared_ptr<spdlog::logger> sysLog, errLog;

	void ResizeImage_C4R(const Ipp8u* pSrc, IppiSize srcSize, Ipp32s srcStep, Ipp8u* pDst, IppiSize dstSize, Ipp32s dstStep);

	void CreateBuffers();
	void DestroyBuffers();
};

}
