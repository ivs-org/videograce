/**
 * ServerAddressDialog.h - Contains enter the server address dialog header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/theme/theme.hpp>
#include <wui/locale/locale.hpp>
#include <wui/window/window.hpp>
#include <wui/control/button.hpp>
#include <wui/control/input.hpp>
#include <wui/control/text.hpp>
#include <wui/control/message.hpp>

#include <wui/config/config.hpp>

#include <memory>
#include <thread>
#include <locale>
#include <codecvt>

namespace Installer
{

class ServerAddressDialog
{
public:
    ServerAddressDialog();
    ~ServerAddressDialog();

    void Run(std::shared_ptr<wui::window> transientWindow, std::function<void(bool ok)> result_callback);

private:
    std::shared_ptr<wui::window> window;

    std::shared_ptr<wui::input> input;
    std::shared_ptr<wui::button> okButton, cancelButton;

    std::shared_ptr<wui::message> messageBox;

    static const int32_t WND_WIDTH = 350, WND_HEIGHT = 140;

    bool ok;

    void ShowIncorrectURLError();
    void OnOK();
};

}
