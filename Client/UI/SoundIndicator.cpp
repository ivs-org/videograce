/**
 * SoundIndicator.cpp - Contains sound indicator impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/SoundIndicator.h>

#include <wui/system/tools.hpp>
#include <wui/window/window.hpp>
#include <wui/theme/theme.hpp>

#include <cmath>

#include <Transport/RTP/RTPPacket.h>

namespace Client
{

SoundIndicator::SoundIndicator()
    : parent_(),
    position_(),
    count(256),
    scale(1.0),
    mutex(),
    data()
{
}

SoundIndicator::~SoundIndicator()
{
}

void SoundIndicator::draw(wui::graphic &gr, const wui::rect &)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto pos = position();

    //gr.draw_rect(pos, wui::make_color(19, 21, 25));

    auto left = pos.left;
    auto center = pos.top + (pos.height() / 2);

    for (int i = 0; i != data.size(); ++i)
    {
        wui::color color = (i != data.size() - 1 ? wui::make_color(54, 183, 41) : wui::make_color(255, 226, 8));
        
        auto val = static_cast<int32_t>(std::round(static_cast<double>(data[i]) * scale));
                
        gr.draw_line({ left + i, center, left + i, center + val }, color, 1);
        gr.draw_line({ left + i, center, left + i, center - val }, color, 1);
        gr.draw_pixel({ left + i, center, left + i, center }, color);
    }
}

void SoundIndicator::set_position(const wui::rect &position__, bool redraw)
{
    count = position__.width();
    scale = (double)position__.height() / 15000000;
    while (data.size() > count)
    {
        data.pop_front();
    }
    update_control_position(position_, position__, redraw, parent_);
}

wui::rect SoundIndicator::position() const
{
    return get_control_position(position_, parent_);
}

void SoundIndicator::set_parent(std::shared_ptr<wui::window> window_)
{
    parent_ = window_;
}

std::weak_ptr<wui::window> SoundIndicator::parent() const
{
    return parent_;
}

void SoundIndicator::clear_parent()
{
    parent_.reset();
}

void SoundIndicator::set_topmost(bool)
{
}

bool SoundIndicator::topmost() const
{
    return false;
}

void SoundIndicator::update_theme(std::shared_ptr<wui::i_theme> theme_)
{

}

bool SoundIndicator::showed() const
{
    return true;
}

bool SoundIndicator::enabled() const
{
    return true;
}

bool SoundIndicator::focused() const
{
    return false;
}

bool SoundIndicator::focusing() const
{
    return false;
}

void SoundIndicator::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
    std::lock_guard<std::mutex> lock(mutex);

    uint64_t value = 0;

    auto &packet = *static_cast<const Transport::RTPPacket*>(&packet_);

    for (uint16_t i = 0; i != packet.payloadSize; i += 2)
    {
        auto sample = *reinterpret_cast<const int16_t*>(packet.payload + i);
        if (sample > 500)
        {
            value += sample;
        }
    }

    data.push_back(value);

    if (data.size() > count)
    {
        data.pop_front();
    }

    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->redraw(position());
    }
}

}
