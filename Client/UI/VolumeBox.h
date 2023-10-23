/**
 * VolumeBox.h - Contains volume box header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/panel.hpp>
#include <wui/control/text.hpp>
#include <wui/control/image.hpp>
#include <wui/control/slider.hpp>

#include <string>
#include <memory>
#include <functional>

namespace Client
{

enum class VolumeBoxMode
{
    Microphone,
    Loudspeaker
};

class VolumeBox : public wui::i_control, public std::enable_shared_from_this<VolumeBox>
{
public:
    VolumeBox(std::function<void(int32_t value, VolumeBoxMode mode, bool enabled)> callback);
    ~VolumeBox();

    virtual void draw(wui::graphic &gr, const wui::rect &) {}

    virtual void set_position(const wui::rect &position, bool redraw = true);
    virtual wui::rect position() const;

    virtual void set_parent(std::shared_ptr<wui::window> window_);
    virtual std::weak_ptr<wui::window> parent() const;
    virtual void clear_parent();

    virtual void set_topmost(bool) {}
    virtual bool topmost() const { return false; }

    virtual void update_theme_control_name(std::string_view) {}
    virtual void update_theme(std::shared_ptr<wui::i_theme> theme_ = nullptr) {}

    virtual void show();
    virtual void hide();
    virtual bool showed() const;

    virtual void enable() {}
    virtual void disable() {}
    virtual bool enabled() const { return true; }

    virtual bool focused() const { return false; }
    virtual bool focusing() const { return false; }

    virtual wui::error get_error() const { return {}; }

    void Acivate(const wui::rect &pos, int32_t value, VolumeBoxMode mode, bool enabled);

private:
    static const int32_t WND_WIDTH = 90, WND_HEIGHT = 250;

    std::function<void(int32_t value, VolumeBoxMode mode, bool enabled)> callback;
    VolumeBoxMode mode;
    bool enabled_;

    std::weak_ptr<wui::window> parent_;
    std::string mySubscriberId;

    std::shared_ptr<wui::panel> panel;
    std::shared_ptr<wui::slider> slider;
    std::shared_ptr<wui::image> image;
    std::shared_ptr<wui::text> text;

    void ReceiveEvent(const wui::event &ev);

    void DrawPanel(wui::graphic &gr);

    void SliderChange(int32_t value);
};

}
