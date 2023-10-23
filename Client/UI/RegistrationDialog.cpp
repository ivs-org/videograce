/**
 * RegistrationDialog.cpp - Contains user registration dialog impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <wui/locale/locale.hpp>

#include <wui/config/config.hpp>
#include <Common/Base64.h>

#include <UI/RegistrationDialog.h>

#include <random>

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/nowide/convert.hpp>

#include <resource.h>

namespace Client
{

RegistrationDialog::RegistrationDialog(std::weak_ptr<wui::window> transientWindow_, std::function<void(RegistrationResult, const std::string&, const std::string&)> readyCallback_)
    : transientWindow(transientWindow_),
    readyCallback(readyCallback_),
    registrator(),
    window(new wui::window()),
    nameText(),
    nameInput(),
    loginText(),
    loginInput(),
    passwordText(),
    passwordInput(),
    passwordConfirmText(),
    passwordConfirmInput(),
    okButton(),
    cancelButton(),
    messageBox()
{
}

RegistrationDialog::~RegistrationDialog()
{
}

void RegistrationDialog::Run(const std::string &serverAddress)
{
    window->set_transient_for(transientWindow.lock());
    nameText = std::shared_ptr<wui::text>(new wui::text(wui::locale("registration_dialog", "name")));
    nameInput = std::shared_ptr<wui::input>(new wui::input(wui::config::get_string("User", "Name", "")));
    loginText = std::shared_ptr<wui::text>(new wui::text(wui::locale("registration_dialog", "login")));
    loginInput = std::shared_ptr<wui::input>(new wui::input());
    passwordText = std::shared_ptr<wui::text>(new wui::text(wui::locale("registration_dialog", "password")));
    passwordInput = std::shared_ptr<wui::input>(new wui::input(wui::config::get_string("Credentials", "Password", ""), wui::input_view::password));
    passwordConfirmText = std::shared_ptr<wui::text>(new wui::text(wui::locale("registration_dialog", "password_confirm")));
    passwordConfirmInput = std::shared_ptr<wui::input>(new wui::input(wui::config::get_string("Credentials", "Password", ""), wui::input_view::password));
    okButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("button", "ok"), std::bind(&RegistrationDialog::OK, this)));
    cancelButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("button", "cancel"), std::bind(&RegistrationDialog::Cancel, this)));
    messageBox = std::shared_ptr<wui::message>(new wui::message(window));

    registrator = std::unique_ptr<Registrator::Registrator>(new Registrator::Registrator());

    registrator->Connect(serverAddress);

    if (!registrator->Connected())
    {
        window->destroy();
        return readyCallback(RegistrationResult::not_connection_to_server, "", "");
    }

    window->add_control(nameText, { 10, 50, WND_WIDTH - 10, 65 });
    window->add_control(nameInput, { 10, 70, WND_WIDTH - 10, 95 });
    window->add_control(loginText, { 10, 180, 120, 205 });
    window->add_control(loginInput, { 130, 180, WND_WIDTH - 10, 205 });
    window->add_control(passwordText, { 10, 215, 120, 240 });
    window->add_control(passwordInput, { 130, 215, WND_WIDTH - 10, 240 });
    window->add_control(passwordConfirmText, { 10, 250, 120, 275 });
    window->add_control(passwordConfirmInput, { 130, 250, WND_WIDTH - 10, 275 });
    window->add_control(okButton, { WND_WIDTH - 230, WND_HEIGHT - 35, WND_WIDTH - 130, WND_HEIGHT - 10 });
    window->add_control(cancelButton, { WND_WIDTH - 110, WND_HEIGHT - 35, WND_WIDTH - 10, WND_HEIGHT - 10 });

#ifdef _WIN32
    wchar_t userName[32767];
    DWORD size = 32767;
    GetUserName(userName, &size);
    loginInput->set_text(boost::nowide::narrow(userName));
#endif
    
    std::mt19937 gen(static_cast<uint32_t>(time(0)));
    std::uniform_int_distribution<> uid(0, 1000000);
    passwordInput->set_text(Common::toBase64(std::to_string(uid(gen))));
    passwordConfirmInput->set_text(passwordInput->text());

    window->set_default_push_control(okButton);

    window->init(wui::locale("registration_dialog", "title"), { -1, -1, WND_WIDTH, WND_HEIGHT }, wui::window_style::dialog, [this]() {
        registrator.reset();
        cancelButton.reset();
        okButton.reset();
        passwordConfirmInput.reset();
        passwordConfirmText.reset();
        passwordInput.reset();
        passwordText.reset();
        loginInput.reset();
        loginText.reset();
        nameInput.reset();
        nameText.reset();
    });
}

void RegistrationDialog::OK()
{
    if (nameInput->text().empty())
    {
        return messageBox->show(wui::locale("message", "name_empty"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { window->set_focused(nameInput); });
    }

    if (loginInput->text().empty())
    {
        return messageBox->show(wui::locale("message", "login_empty"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { window->set_focused(loginInput); });
    }

    if (passwordInput->text().empty())
    {
        return messageBox->show(wui::locale("message", "password_empty"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { window->set_focused(passwordInput); });
    }

    if (passwordInput->text() != passwordConfirmInput->text())
    {
        return messageBox->show(wui::locale("message", "password_and_confirm_not_equal"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { window->set_focused(passwordConfirmInput); });
    }

    if (registrator->Connected())
    {
        Register();
    }
}

void RegistrationDialog::Cancel()
{
    window->destroy();
}

void RegistrationDialog::Register()
{
    auto result = registrator->Register(nameInput->text(), "", loginInput->text(), passwordInput->text());
    switch (result)
    {
        case Proto::USER_UPDATE_RESPONSE::Result::OK:
            messageBox->show(wui::locale("message", "registration_success"),
                wui::locale("message", "title_notification"),
                wui::message_icon::information,
                wui::message_button::ok,
                [this](wui::message_result) {
                    auto login = loginInput->text(), password = passwordInput->text();
                    window->destroy();
                    readyCallback(RegistrationResult::ok, login, password);
            });
        break;
        case Proto::USER_UPDATE_RESPONSE::Result::DuplicateName:
            messageBox->show(wui::locale("message", "duplicate_name"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok, [this](wui::message_result) { window->set_focused(nameInput); });
        break;
        case Proto::USER_UPDATE_RESPONSE::Result::DuplicateLogin:
            messageBox->show(wui::locale("message", "duplicate_credentionals"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok, [this](wui::message_result) { window->set_focused(loginInput); });
        break;
        case Proto::USER_UPDATE_RESPONSE::Result::RegistrationDenied:
            messageBox->show(wui::locale("message", "registration_denied"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok,
                [this](wui::message_result) {
                    window->destroy();
                    readyCallback(RegistrationResult::other_error, "", "");
            });
        break;
    }
}

}
