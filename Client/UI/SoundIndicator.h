/**
 * SoundIndicator.h - Contains sound indicator header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/control/i_control.hpp>
#include <wui/common/rect.hpp>

#include <Transport/ISocket.h>

#include <deque>
#include <mutex>

namespace Client
{

class SoundIndicator : public wui::i_control, public std::enable_shared_from_this<SoundIndicator>, public Transport::ISocket
{
public:
    SoundIndicator();
    ~SoundIndicator();

    /// wui::i_control impl
    virtual void draw(wui::graphic &gr, const wui::rect &);

    virtual void set_position(const wui::rect &position, bool redraw = true);
    virtual wui::rect position() const;

    virtual void set_parent(std::shared_ptr<wui::window> window_);
    virtual std::weak_ptr<wui::window> parent() const;
    virtual void clear_parent();

    virtual void set_topmost(bool);
    virtual bool topmost() const;

    virtual void update_theme_control_name(std::string_view) {}
    virtual void update_theme(std::shared_ptr<wui::i_theme> theme_ = nullptr);

    virtual void show() {}
    virtual void hide() {}
    virtual bool showed() const;

    virtual void enable() {}
    virtual void disable() {}
    virtual bool enabled() const;

    virtual bool focused() const;
    virtual bool focusing() const;

    virtual wui::error get_error() const { return {}; }

    /// Transport::ISocket impl
    virtual void Send(const Transport::IPacket &packet_, const Transport::Address *) final;

private:
    std::weak_ptr<wui::window> parent_;
    wui::rect position_;

    uint16_t count;

    std::mutex mutex;
    std::deque<uint64_t> data;
};

}
