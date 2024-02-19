/**
 * DemonstrationWindow.h - Contains screen demonstation window header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/button.hpp>
#include <wui/control/scroll.hpp>

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
    static constexpr int32_t BTN_SIZE = 32;

    RendererSession::RendererVideoSession& rvs;
    Controller::IController& controller;

    std::shared_ptr<wui::window> window;

    std::shared_ptr<wui::button> rcButton, scaleButton;

    std::shared_ptr<wui::scroll> verScroll, horScroll;

    mt::timer timer_;

    double scaleFactor;
    int32_t shiftLeft, shiftTop;

    void ReceiveEvents(const wui::event &ev);

    void Redraw();

    void OnRemoteControl();
    void OnScale();

    void Resize(int32_t width, int32_t height);

    void OnVertScroll(wui::scroll_state ss, int32_t v);
    void OnHorScroll(wui::scroll_state ss, int32_t v);

    void UpdateRendererPos(int32_t width, int32_t height);
    void MoveRenderer(int32_t left, int32_t top);

    int32_t NormMouseX(int32_t input) const;
    int32_t NormMouseY(int32_t input) const;
};

}
