/**
 * DemonstrationWindow.h - Contains screen demonstation window header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#pragma once

#include <wui/window/window.hpp>

#include <string>
#include <memory>
#include <functional>

#include <mt/timer.h>

#include <RendererSession/IRendererVideoSession.h>

namespace Client
{

class DemonstrationWindow
{
public:
    DemonstrationWindow(RendererSession::IRendererVideoSession &rvs, const wui::rect &screenSize);
    ~DemonstrationWindow();

private:
    std::shared_ptr<wui::window> window;

    RendererSession::IRendererVideoSession &rvs;

    mt::timer timer_;

    void ReceiveEvents(const wui::event &ev);

    void Redraw();
};

}
