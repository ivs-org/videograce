/**
 * CredentialsDialog.h - Contains credentials dialog header
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

#include <Controller/IController.h>

#include <UI/RegistrationDialog.h>

namespace Client
{

class CredentialsDialog
{
public:
    CredentialsDialog(std::weak_ptr<wui::window> mainFrame, Controller::IController &controller_, std::function<void()> closeCallback);
    ~CredentialsDialog();

    void Run();

    std::string login, password;

private:
    static const int32_t WND_WIDTH = 400, WND_HEIGHT = 400;

    std::weak_ptr<wui::window> mainFrame;
    Controller::IController &controller;
    std::function<void()> closeCallback;

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::button> cloudButton, localButton;
    std::shared_ptr<wui::text> serverText;
    std::shared_ptr<wui::input> serverInput;
    std::shared_ptr<wui::text> loginText;
    std::shared_ptr<wui::input> loginInput;
    std::shared_ptr<wui::text> passwordText;
    std::shared_ptr<wui::input> passwordInput;
    std::shared_ptr<wui::button> rememberCheck;
    std::shared_ptr<wui::button> okButton, cancelButton;
    std::shared_ptr<wui::button> registrationLink;
    std::shared_ptr<wui::message> messageBox;

    std::shared_ptr<RegistrationDialog> registrationDialog;

    bool ok;

    std::string localServerAddress;

    void Cloud();
    void Local();

    void OK();
    void Cancel();
    void Registration();

    bool CheckServerAddress();
    void UpdateServerAddress();
    
    void RegistrationEndCallback(RegistrationResult result, std::string_view login, std::string_view password);
};

}
