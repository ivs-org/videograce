/**
 * RegistrationDialog.h - Contains user registration dialog header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/text.hpp>
#include <wui/control/input.hpp>
#include <wui/control/select.hpp>
#include <wui/control/button.hpp>
#include <wui/control/message.hpp>

#include <Registrator/Registrator.h>

#include <string>
#include <memory>
#include <functional>

namespace Client
{

enum class RegistrationResult
{
    ok,
    not_connection_to_server,
    other_error
};

class RegistrationDialog
{
public:
    RegistrationDialog(std::weak_ptr<wui::window> transientWindow, std::function<void(RegistrationResult, const std::string&, const std::string&)> readyCallback);
    ~RegistrationDialog();

    void Run(std::string_view serverAddress = "");

private:
    static const int32_t WND_WIDTH = 390, WND_HEIGHT = 390;

    std::weak_ptr<wui::window> transientWindow;

    std::function<void(RegistrationResult, const std::string&, const std::string&)> readyCallback;

    std::unique_ptr<Registrator::Registrator> registrator;
    
    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::text> nameText;
    std::shared_ptr<wui::input> nameInput;
    std::shared_ptr<wui::text> loginText;
    std::shared_ptr<wui::input> loginInput;
    std::shared_ptr<wui::text> passwordText;
    std::shared_ptr<wui::input> passwordInput;
    std::shared_ptr<wui::text> passwordConfirmText;
    std::shared_ptr<wui::input> passwordConfirmInput;
    std::shared_ptr<wui::button> okButton, cancelButton;
    std::shared_ptr<wui::message> messageBox;

    void OK();
    void Cancel();

    void Register();
};

}
