/**
 * DemonstrationWindow.h - Contains screen demonstation window header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/button.hpp>

#include <string>
#include <memory>
#include <functional>

#include <mt/timer.h>

#include <RendererSession/RendererVideoSession.h>

#include <Controller/IController.h>

namespace Client
{

class DemonstrationWindow
{
public:
    DemonstrationWindow(RendererSession::RendererVideoSession &rvs,
        Controller::IController& controller_,
        const wui::rect &screenSize);
    ~DemonstrationWindow();

    void EnableRC(bool yes);

    int64_t GetClientId() const;

private:
    static const int32_t BTN_SIZE = 32;

    RendererSession::RendererVideoSession& rvs;
    Controller::IController& controller;

    std::shared_ptr<wui::window> window;

    std::shared_ptr<wui::button> rcButton, scaleButton;

    mt::timer timer_;

    void ReceiveEvents(const wui::event &ev);

    void Redraw();

    void OnRemoteControl();
    void OnScale();
};

}
