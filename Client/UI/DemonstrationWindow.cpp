/**
 * DemonstrationWindow.cpp - Contains screen demonstration window impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#include <UI/DemonstrationWindow.h>

#include <wui/locale/locale.hpp>
#include <wui/theme/theme.hpp>
#include <wui/system/tools.hpp>

#include <resource.h>

namespace Client
{

DemonstrationWindow::DemonstrationWindow(RendererSession::IRendererVideoSession &rvs_, const wui::rect &screenSize)
    : window(new wui::window()),
    rvs(rvs_),
    timer_(std::bind(&DemonstrationWindow::Redraw, this))
{
    window->subscribe(std::bind(&DemonstrationWindow::ReceiveEvents, this, std::placeholders::_1),
        static_cast<wui::event_type>(static_cast<int32_t>(wui::event_type::internal) | static_cast<int32_t>(wui::event_type::system) |
            static_cast<int32_t>(wui::event_type::keyboard) | static_cast<int32_t>(wui::event_type::mouse)));

    window->add_control(rvs.GetControl(), { 0 });

    auto resolution = rvs.GetResolution();
    auto resoutionWidth = Video::GetValues(resolution).width;
    auto resoutionHeight = Video::GetValues(resolution).height;

    int32_t wndWidth = resoutionWidth < screenSize.width() ? resoutionWidth : screenSize.width(),
        wndHeight = resoutionHeight < screenSize.height() ? resoutionHeight : screenSize.height();

    window->init(rvs.GetName() + " - " + wui::locale("device", "screen_capturer"), { 20, 20, wndWidth, wndHeight },
        static_cast<wui::window_style>(static_cast<int32_t>(wui::window_style::title_showed) | static_cast<int32_t>(wui::window_style::expand_button) | static_cast<int32_t>(wui::window_style::minimize_button) | static_cast<int32_t>(wui::window_style::resizable) | static_cast<int32_t>(wui::window_style::moving)),
        [this]() {
        timer_.stop();
        window.reset();
    });
}

DemonstrationWindow::~DemonstrationWindow()
{
    window.reset();
}

wui::rect FitRenderer(Video::Resolution resolution, int32_t width, int32_t height)
{
    wui::rect out = { 0 };

    if (resolution == 0)
    {
        return out;
    }

    Video::ResolutionValues rv = Video::GetValues(resolution);

    int32_t X = width,
        Y = height,
        W = rv.width,
        H = rv.height;

    if ((static_cast<double>(H) / W) > 0.57)
    {
        W = static_cast<int32_t>(static_cast<double>(H) / 0.56);
    }

    int32_t x = 0, y = 0;

    x = X;
    y = H * X / W;
    if (y > Y)
    {
        y = Y;
        x = W * Y / H;
        out.left = (X - x) / 2;
        out.top = 0;
    }
    else
    {
        out.left = 0;
        out.top = (Y - y) / 2;
    }

    out.right = out.left + x;
    out.bottom = out.top + y;

    return out;
}

void DemonstrationWindow::ReceiveEvents(const wui::event &ev)
{
    switch (ev.type)
    {
        case wui::event_type::internal:
        {
            switch (ev.internal_event_.type)
            {
                case wui::internal_event_type::window_created:
                    timer_.start(40);
                break;
                case wui::internal_event_type::size_changed:
                case wui::internal_event_type::window_expanded:
                    rvs.GetControl()->set_position(FitRenderer(rvs.GetResolution(), ev.internal_event_.x, ev.internal_event_.y));
                break;
            }
        }
    }
}

void DemonstrationWindow::Redraw()
{
    if (window)
    {
        auto pos = window->position();
        window->redraw({ 0, 0, pos.width(), pos.height() });
    }
}

}
