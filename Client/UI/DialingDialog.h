/**
 * DialingDialog.h - Contains dialing dialog header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/text.hpp>
#include <wui/control/image.hpp>
#include <wui/control/button.hpp>

#include <string>
#include <memory>
#include <functional>

namespace Client
{

class DialingDialog
{
public:
    DialingDialog(std::weak_ptr<wui::window> transientWindow, std::function<void()> cancelCallback);
    ~DialingDialog();

    void Run(std::string_view subscriber);
    void End();

private:
    static const int32_t WND_WIDTH = 300, WND_HEIGHT = 150;

    std::weak_ptr<wui::window> transientWindow;
    std::function<void()> cancelCallback;

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::text> text;
    std::shared_ptr<wui::image> image;
    std::shared_ptr<wui::button> cancelButton;
    
    void Cancel();
};

}
