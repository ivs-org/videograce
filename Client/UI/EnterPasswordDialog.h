/**
 * EnterPasswordDialog.h - Contains entering password dialog header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/text.hpp>
#include <wui/control/input.hpp>
#include <wui/control/button.hpp>
#include <wui/control/message.hpp>

#include <string>
#include <memory>
#include <functional>

namespace Client
{

class EnterPasswordDialog
{
public:
    EnterPasswordDialog(std::weak_ptr<wui::window> transientWindow, std::function<void(const std::string&)> readyCallback);
    ~EnterPasswordDialog();

    void Run();

private:
    static const int32_t WND_WIDTH = 300, WND_HEIGHT = 150;

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::input> passwordInput;
    std::shared_ptr<wui::button> okButton, cancelButton;
};

}
