/**
 * BusyBox.h - Contains busy box header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/text.hpp>
#include <wui/control/image.hpp>

#include <mt/timer.h>

#include <string>
#include <memory>
#include <functional>

namespace Client
{

class BusyBox
{
public:
    BusyBox(std::weak_ptr<wui::window> transientWindow);
    ~BusyBox();

    void Run(const std::string &text);
    void End();

private:
    static const int32_t WND_WIDTH = 240, WND_HEIGHT = 290;

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::image> image;
    std::shared_ptr<wui::text> text;

    mt::timer timer_;

    int32_t currentImage;
    void UpdateImage(const wui::event&);
};

}
