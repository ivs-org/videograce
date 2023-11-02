/**
 * VolumeBox.cpp - Contains volume box impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/VolumeBox.h>

#include <wui/locale/locale.hpp>
#include <wui/system/tools.hpp>
#include <wui/theme/theme.hpp>

#include <resource.h>

namespace Client
{

VolumeBox::VolumeBox(std::function<void(int32_t value, VolumeBoxMode mode, bool enabled)> callback_)
    : callback(callback_), mode(VolumeBoxMode::Microphone), enabled_(true),
    parent_(),
    mySubscriberId(),
    panel(new wui::panel(std::bind(&VolumeBox::DrawPanel, this, std::placeholders::_1))),
    slider(new wui::slider(0, 100, 100, std::bind(&VolumeBox::SliderChange, this, std::placeholders::_1), wui::slider_orientation::vertical)),
    image(new wui::image(IMG_TB_LOUDSPEAKER)),
    text(new wui::text("100", wui::hori_alignment::center))
{
    panel->set_topmost(true);
    slider->set_topmost(true);
    image->set_topmost(true);
    text->set_topmost(true);

    hide();
}

VolumeBox::~VolumeBox()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(panel);
        parent__->remove_control(slider);
        parent__->remove_control(image);
        parent__->remove_control(text);
    }
}

void VolumeBox::set_position(const wui::rect &pos, bool redraw)
{
    panel->set_position(pos, redraw);
    slider->set_position({ pos.left + 20, pos.top + 50, pos.right - 20, pos.bottom - 40 }, redraw);
    image->set_position({ pos.left + 29, pos.top + 10, pos.right - 29, pos.top + 42 }, redraw);
    text->set_position({ pos.left + 10, pos.bottom - 25, pos.right - 10, pos.bottom - 10 }, redraw);
}

wui::rect VolumeBox::position() const
{
    return panel->position();
}

void VolumeBox::set_parent(std::shared_ptr<wui::window> window)
{
    parent_ = window;

    if (window)
    {
        window->add_control(panel, { 0 });
        window->add_control(slider, { 0 });
        window->add_control(image, { 0 });
        window->add_control(text, { 0 });

        mySubscriberId = window->subscribe(std::bind(&VolumeBox::ReceiveEvent, this, std::placeholders::_1),
            static_cast<wui::event_type>(static_cast<uint32_t>(wui::event_type::mouse) | static_cast<uint32_t>(wui::event_type::keyboard)));
    }
}

std::weak_ptr<wui::window> VolumeBox::parent() const
{
    return parent_;
}

void VolumeBox::clear_parent()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        panel->clear_parent();
        slider->clear_parent();
        image->clear_parent();
        text->clear_parent();

        parent__->unsubscribe(mySubscriberId);
    }

    parent_.reset();
}

void VolumeBox::show()
{
    if (panel->showed())
    {
        return;
    }

    panel->show();
    slider->show();
    image->show();
    text->show();
}

void VolumeBox::hide()
{
    panel->hide();
    slider->hide();
    image->hide();
    text->hide();
}

bool VolumeBox::showed() const
{
    return panel->showed();
}

void VolumeBox::Acivate(const wui::rect &pos_, int32_t value, VolumeBoxMode mode_, bool enabled__)
{
    mode = mode_;
    enabled_ = enabled__;

    switch (mode)
    {
        case VolumeBoxMode::Microphone:
            image->change_image(enabled_ ? IMG_TB_MICROPHONE : IMG_TB_MICROPHONE_DISABLED);
        break;
        case VolumeBoxMode::Loudspeaker:
            image->change_image(enabled_ ? IMG_TB_LOUDSPEAKER : IMG_TB_LOUDSPEAKER_DISABLED);
        break;
    }

    slider->set_value(value);
    text->set_text(std::to_string(value));

    auto pos = get_popup_position(parent_, pos_, { 0, 0, WND_WIDTH, WND_HEIGHT }, 2);
    set_position(pos, true);
    
    show();

    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->set_focused(slider);
    }
}

void VolumeBox::ReceiveEvent(const wui::event &ev)
{
    if (!panel->showed())
    {
        return;
    }

    switch (ev.type)
    {
        case wui::event_type::mouse:
            if (ev.mouse_event_.type == wui::mouse_event_type::left_up || ev.mouse_event_.type == wui::mouse_event_type::right_up)
            {
                if (!panel->position().in({ ev.mouse_event_.x, ev.mouse_event_.y, ev.mouse_event_.x, ev.mouse_event_.y }))
                {
                    hide();
                }
                if (image->position().in({ ev.mouse_event_.x, ev.mouse_event_.y, ev.mouse_event_.x, ev.mouse_event_.y }))
                {
                    enabled_ = !enabled_;
                    switch (mode)
                    {
                        case VolumeBoxMode::Microphone:
                            image->change_image(enabled_ ? IMG_TB_MICROPHONE : IMG_TB_MICROPHONE_DISABLED);
                        break;
                        case VolumeBoxMode::Loudspeaker:
                            image->change_image(enabled_ ? IMG_TB_LOUDSPEAKER : IMG_TB_LOUDSPEAKER_DISABLED);
                        break;
                    }
                    callback(slider->get_value(), mode, enabled_);
                }
            }
        break;
        case wui::event_type::keyboard:
            if (ev.keyboard_event_.type == wui::keyboard_event_type::up)
            {
                if (ev.keyboard_event_.key[0] == wui::vk_esc)
                {
                    hide();
                }
            }
        break;
    }
}

void VolumeBox::DrawPanel(wui::graphic &gr)
{
    gr.draw_rect(panel->position(), wui::theme_color(wui::window::tc, wui::window::tv_border), wui::theme_color(wui::window::tc, wui::window::tv_background), 1, 0);
}

void VolumeBox::SliderChange(int32_t value)
{
    text->set_text(std::to_string(value));
    callback(value, mode, enabled_);
}

}
