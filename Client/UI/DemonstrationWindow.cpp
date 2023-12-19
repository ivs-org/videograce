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

static const wui::window_style full_style = wui::flags_map<wui::window_style>(4,
    wui::window_style::expand_button,
    wui::window_style::minimize_button,
    wui::window_style::resizable,
    wui::window_style::moving);

DemonstrationWindow::DemonstrationWindow(RendererSession::RendererVideoSession &rvs_,
    Controller::IController& controller_,
    const wui::rect &screenSize)
    : rvs(rvs_), controller(controller_),
    window(new wui::window()),
    rcButton(new wui::button(wui::locale("screen_capturer", "enable_remote_control"), std::bind(&DemonstrationWindow::OnRemoteControl, this), wui::button_view::image, IMG_TB_REMOTE_CONTROL, BTN_SIZE, wui::button::tc_tool)),
    scaleButton(new wui::button(wui::locale("screen_capturer", "scale_rc_to_window"), std::bind(&DemonstrationWindow::OnScale, this), wui::button_view::image, IMG_TB_SCALE, BTN_SIZE, wui::button::tc_tool)),
    verScroll(new wui::scroll(0, 0, wui::orientation::vertical, std::bind(&DemonstrationWindow::OnVertScroll, this, std::placeholders::_1, std::placeholders::_2))),
    horScroll(new wui::scroll(0, 0, wui::orientation::horizontal, std::bind(&DemonstrationWindow::OnHorScroll, this, std::placeholders::_1, std::placeholders::_2))),
    timer_(std::bind(&DemonstrationWindow::Redraw, this)),
    scaleFactor(1.0),
    shiftLeft(0), shiftTop(0)
{
    window->subscribe(std::bind(&DemonstrationWindow::ReceiveEvents, this, std::placeholders::_1),
        wui::flags_map<wui::event_type>(4,
            wui::event_type::internal,
            wui::event_type::system,
            wui::event_type::keyboard,
            wui::event_type::mouse));

    scaleButton->turn(true);

    window->add_control(rvs.GetControl(), { 0 });

    window->add_control(rcButton, { 0 });
    window->add_control(scaleButton, { 100 + BTN_SIZE, 0, (BTN_SIZE * 2) + 100, BTN_SIZE });
    window->add_control(verScroll, { 0 });
    window->add_control(horScroll, { 0 });
    verScroll->hide();
    horScroll->hide();

    auto rv = Video::GetValues(rvs.GetResolution());
    int32_t wndWidth = rv.width < screenSize.width() ? rv.width : screenSize.width(),
        wndHeight = rv.height < screenSize.height() ? rv.height : screenSize.height();

    window->init(std::string(rvs.GetName()) + " - " + wui::locale("device", "screen_capturer"), { -1, -1, wndWidth, wndHeight },
        full_style,
        [this]() {
        timer_.stop();
    });
}

DemonstrationWindow::~DemonstrationWindow()
{
    window->destroy();
}

void DemonstrationWindow::UpdateRendererPos(int32_t width, int32_t height)
{
    auto resolution = rvs.GetResolution();
    if (resolution == 0)
    {
        return;
    }

    wui::rect out = { 0 };

    auto rv = Video::GetValues(resolution);

    double X = width,
        Y = height,
        W = rv.width,
        H = rv.height;

    double x = 0, y = 0;

    x = X;
    y = H * X / W;
    if (y > Y)
    {
        y = Y;
        x = W * Y / H;
        
        out.left = static_cast<int32_t>((X - x) / 2);
        out.top = 0;

        scaleFactor = Y / H;
    }
    else
    {
        out.left = 0;
        out.top = static_cast<int32_t>((Y - y) / 2);

        scaleFactor = X / W;
    }

    shiftLeft = out.left;
    shiftTop = out.top;

    out.right = out.left + static_cast<int32_t>(x);
    out.bottom = out.top + static_cast<int32_t>(y);

    rvs.GetControl()->set_position(out);
}

int32_t DemonstrationWindow::NormMouseX(int32_t input) const
{
    return static_cast<int32_t>((input - shiftLeft) / scaleFactor);
}

int32_t DemonstrationWindow::NormMouseY(int32_t input) const
{
    return static_cast<int32_t>((input - shiftTop) / scaleFactor);
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
                    Resize(ev.internal_event_.x, ev.internal_event_.y);
                break;
            }
        }
        case wui::event_type::mouse:
            switch (ev.mouse_event_.type)
            {
                case wui::mouse_event_type::move:
                    rvs.MouseMove        (NormMouseX(ev.mouse_event_.x), NormMouseY(ev.mouse_event_.y));
                break;
                case wui::mouse_event_type::left_down:
                    rvs.MouseLeftDown    (NormMouseX(ev.mouse_event_.x), NormMouseY(ev.mouse_event_.y));
                break;
                case wui::mouse_event_type::left_up:
                    rvs.MouseLeftUp      (NormMouseX(ev.mouse_event_.x), NormMouseY(ev.mouse_event_.y));
                break;
                case wui::mouse_event_type::center_down:
                    rvs.MouseCenterDown  (NormMouseX(ev.mouse_event_.x), NormMouseY(ev.mouse_event_.y));
                break;
                case wui::mouse_event_type::center_up:
                    rvs.MouseCenterUp    (NormMouseX(ev.mouse_event_.x), NormMouseY(ev.mouse_event_.y));
                break;
                case wui::mouse_event_type::right_down:
                    rvs.MouseRightDown   (NormMouseX(ev.mouse_event_.x), NormMouseY(ev.mouse_event_.y));
                break;
                case wui::mouse_event_type::right_up:
                    rvs.MouseRightUp     (NormMouseX(ev.mouse_event_.x), NormMouseY(ev.mouse_event_.y));
                break;
                case wui::mouse_event_type::left_double:
                    rvs.MouseLeftDblClick(NormMouseX(ev.mouse_event_.x), NormMouseY(ev.mouse_event_.y));
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
        window->set_style(full_style);
    }
}

void DemonstrationWindow::OnRemoteControl()
{
    auto rcEnabled = !rcButton->turned();
    if (!rcEnabled)
    {
        rcButton->turn(false);
        window->set_style(full_style);
    }
    controller.SendMemberAction(std::vector<int64_t>{ rvs.GetClientId() }, rcEnabled ? Proto::MEMBER_ACTION::Action::EnableRemoteControl : Proto::MEMBER_ACTION::Action::DisableRemoteControl);
}

void DemonstrationWindow::OnScale()
{
    scaleButton->turn(!scaleButton->turned());

    if (scaleButton->turned())
    {
        horScroll->hide();
        verScroll->hide();
    }
    
    auto pos = window->position();
    Resize(pos.width(), pos.height());
}

int64_t DemonstrationWindow::GetClientId() const
{
    return rvs.GetClientId();
}

void DemonstrationWindow::Resize(int32_t width, int32_t height)
{
    int32_t btnWidth = BTN_SIZE * 2;
    int32_t left = (width - btnWidth) / 2;
    rcButton->set_position({ left, 0, left + BTN_SIZE, BTN_SIZE });
    scaleButton->set_position({ left + BTN_SIZE, 0, left + (BTN_SIZE * 2), BTN_SIZE });
    
    verScroll->set_position({ width - 14, 30, width, height });
    horScroll->set_position({ 0, height - 14, width - 14, height });

    if (scaleButton->turned())
    {
        UpdateRendererPos(width, height);
    }
    else
    {
        auto rv = Video::GetValues(rvs.GetResolution());

        scaleFactor = 1.0;

        if (rv.width < width)
        {
            shiftLeft = (width - rv.width) / 2;
        }
        if (rv.height < height)
        {
            shiftTop = (height - rv.height) / 2;
        }

        rvs.GetControl()->set_position({ shiftLeft,
            shiftTop,
            shiftLeft + rv.width,
            shiftTop + rv.height });

        if (rv.width > width)
        {
            auto area = rv.width - width;
            if (area < horScroll->get_scroll_pos())
            {
                horScroll->set_scroll_pos(0);
            }
            horScroll->set_area(area);
            horScroll->show();
        }
        else
        {
            horScroll->hide();
        }
        if (rv.height > height)
        {
            auto area = rv.height - height;
            if (area < verScroll->get_scroll_pos())
            {
                verScroll->set_scroll_pos(0);
            }
            verScroll->set_area(area);
            verScroll->show();
        }
        else
        {
            verScroll->hide();
        }
    }
}

void DemonstrationWindow::OnVertScroll(wui::scroll_state ss, int32_t v)
{
    MoveRenderer(-1, -v);
}

void DemonstrationWindow::OnHorScroll(wui::scroll_state ss, int32_t v)
{
    MoveRenderer(-v, -1);
}

void DemonstrationWindow::MoveRenderer(int32_t left, int32_t top)
{
    if (left != -1)
    {
        shiftLeft = left;
    }
    if (top != -1)
    {
        shiftTop = top;
    }

    auto resolution = rvs.GetResolution();
    if (resolution == 0)
    {
        return;
    }

    auto rv = Video::GetValues(resolution);

    rvs.GetControl()->set_position({ shiftLeft,
        shiftTop,
        shiftLeft + rv.width,
        shiftTop + rv.height });
}

}
