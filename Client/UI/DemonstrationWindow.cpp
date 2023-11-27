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
#include <wui/common/flag_helpers.hpp>

#include <Proto/CmdMemberAction.h>

#include <resource.h>

namespace Client
{

DemonstrationWindow::DemonstrationWindow(RendererSession::RendererVideoSession &rvs_,
    Controller::IController& controller_,
    const wui::rect &screenSize)
    : rvs(rvs_), controller(controller_),
    window(new wui::window()),
    rcButton(new wui::button(wui::locale("screen_capturer", "enable_remote_control"), std::bind(&DemonstrationWindow::OnRemoteControl, this), wui::button_view::image, IMG_TB_REMOTE_CONTROL, BTN_SIZE, wui::button::tc_tool)),
    scaleButton(new wui::button(wui::locale("screen_capturer", "scale_rc_to_100_percent"), std::bind(&DemonstrationWindow::OnScale, this), wui::button_view::image, IMG_TB_SCALE, BTN_SIZE, wui::button::tc_tool)),
    timer_(std::bind(&DemonstrationWindow::Redraw, this))
{
    window->subscribe(std::bind(&DemonstrationWindow::ReceiveEvents, this, std::placeholders::_1),
        wui::flags_map<wui::event_type>(4,
            wui::event_type::internal,
            wui::event_type::system,
            wui::event_type::keyboard,
            wui::event_type::mouse));

    window->add_control(rvs.GetControl(), { 0 });

    window->add_control(rcButton, { 0 });
    window->add_control(scaleButton, { 100 + BTN_SIZE, 0, (BTN_SIZE * 2) + 100, BTN_SIZE });

    auto resolution = rvs.GetResolution();
    auto resoutionWidth = Video::GetValues(resolution).width;
    auto resoutionHeight = Video::GetValues(resolution).height;

    int32_t wndWidth = resoutionWidth < screenSize.width() ? resoutionWidth : screenSize.width(),
        wndHeight = resoutionHeight < screenSize.height() ? resoutionHeight : screenSize.height();

    window->init(rvs.GetName() + " - " + wui::locale("device", "screen_capturer"), { 20, 20, wndWidth, wndHeight },
        wui::flags_map<wui::window_style>(5,
            wui::window_style::title_showed,
            wui::window_style::expand_button,
            wui::window_style::minimize_button,
            wui::window_style::resizable,
            wui::window_style::moving),
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
                {
                    rvs.GetControl()->set_position(FitRenderer(rvs.GetResolution(), ev.internal_event_.x, ev.internal_event_.y));

                    int32_t btnWidth = BTN_SIZE * 2;
                    int32_t left = (ev.internal_event_.x - btnWidth) / 2;
                    rcButton->set_position({ left, 0, left + BTN_SIZE, BTN_SIZE });
                    scaleButton->set_position({ left + BTN_SIZE, 0, left + (BTN_SIZE * 2), BTN_SIZE });
                }
                break;
            }
        }
        case wui::event_type::mouse:
            switch (ev.mouse_event_.type)
            {
                case wui::mouse_event_type::move:
                    rvs.MouseMove(ev.mouse_event_.x, ev.mouse_event_.y);
                break;
                case wui::mouse_event_type::left_down:
                    rvs.MouseLeftDown(ev.mouse_event_.x, ev.mouse_event_.y);
                break;
                case wui::mouse_event_type::left_up:
                    rvs.MouseLeftUp(ev.mouse_event_.x, ev.mouse_event_.y);
                break;
                case wui::mouse_event_type::center_down:
                    rvs.MouseCenterDown(ev.mouse_event_.x, ev.mouse_event_.y);
                break;
                case wui::mouse_event_type::center_up:
                    rvs.MouseCenterUp(ev.mouse_event_.x, ev.mouse_event_.y);
                break;
                case wui::mouse_event_type::right_down:
                    rvs.MouseRightDown(ev.mouse_event_.x, ev.mouse_event_.y);
                break;
                case wui::mouse_event_type::right_up:
                    rvs.MouseRightUp(ev.mouse_event_.x, ev.mouse_event_.y);
                break;
                case wui::mouse_event_type::left_double:
                    rvs.MouseLeftDblClick(ev.mouse_event_.x, ev.mouse_event_.y);
                break;
                case wui::mouse_event_type::wheel:
                    rvs.MouseWheel(ev.mouse_event_.wheel_delta);
                break;
            }
        break;
        case wui::event_type::keyboard:
            switch (ev.keyboard_event_.type)
            {
                case wui::keyboard_event_type::down:
                    rvs.KeyDown(ev.keyboard_event_.modifier, ev.keyboard_event_.key, ev.keyboard_event_.key_size);
                break;
                case wui::keyboard_event_type::up:
                    rvs.KeyUp(ev.keyboard_event_.modifier, ev.keyboard_event_.key, ev.keyboard_event_.key_size);
                break;
            }
        break;
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

void DemonstrationWindow::EnableRC(bool yes)
{
    rcButton->turn(yes);
    if (yes)
    {
        window->set_style(wui::flags_map<wui::window_style>(3,
            wui::window_style::expand_button,
            wui::window_style::minimize_button,
            wui::window_style::resizable));
    }
    else
    {
        window->set_style(wui::flags_map<wui::window_style>(5,
            wui::window_style::title_showed,
            wui::window_style::expand_button,
            wui::window_style::minimize_button,
            wui::window_style::resizable,
            wui::window_style::moving));
    }
}

void DemonstrationWindow::OnRemoteControl()
{
    auto rcEnabled = !rcButton->turned();
    if (!rcEnabled)
    {
        rcButton->turn(false);
    }
    controller.SendMemberAction(std::vector<int64_t>{ rvs.GetClientId() }, rcEnabled ? Proto::MEMBER_ACTION::Action::EnableRemoteControl : Proto::MEMBER_ACTION::Action::DisableRemoteControl);
}

void DemonstrationWindow::OnScale()
{

}

int64_t DemonstrationWindow::GetClientId() const
{
    return rvs.GetClientId();
}

}
