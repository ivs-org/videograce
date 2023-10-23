/**
 * MiniWindow.h - Contains mini window header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/image.hpp>

#include <string>
#include <memory>
#include <functional>

#include <mt/timer.h>

namespace Client
{

class MiniWindow
{
public:
    MiniWindow(std::weak_ptr<wui::window> mainWindow);
    ~MiniWindow();

    void Run();
    void End();

    void ShowRenderer(std::shared_ptr<wui::i_control> renderer);

    void SetCameraState(bool enabled);
    void SetMicrophoneState(bool enabled);

private:
    static const int32_t WND_WIDTH = 266, WND_HEIGHT = 148;

    std::weak_ptr<wui::window> mainWindow;

    std::shared_ptr<wui::window> window;

    std::shared_ptr<wui::image> cameraImage, microphoneImage;

    mt::timer timer_;

    bool cameraEnabled, microphoneEnabled;

    int32_t blinkTickCount;

    void ReceiveEvents(const wui::event &ev);

    void Redraw();
};

}
