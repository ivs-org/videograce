/**
 * CredentialsDialog.cpp - Contains credentials dialog impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <wui/locale/locale.hpp>
#include <wui/theme/theme.hpp>

#include <wui/config/config.hpp>

#include <UI/CredentialsDialog.h>

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <resource.h>

#include <Version.h>

namespace Client
{

CredentialsDialog::CredentialsDialog(std::weak_ptr<wui::window> mainFrame_, Controller::IController &controller_, std::function<void()> closeCallback_)
    : login(), password(),
    mainFrame(mainFrame_),
    controller(controller_),
    closeCallback(closeCallback_),
    window(new wui::window()),
    cloudButton(), localButton(),
    serverText(),
    serverInput(),
    loginText(),
    loginInput(),
    passwordText(),
    passwordInput(),
    rememberCheck(),
    okButton(),
    cancelButton(),
    registrationLink(),
    messageBox(),
    registrationDialog(),
    ok(false)
{
}

CredentialsDialog::~CredentialsDialog()
{
}

void CredentialsDialog::Run()
{
    window->set_transient_for(mainFrame.lock());
    cloudButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("credentials_dialog", "connection_cloud"), std::bind(&CredentialsDialog::Cloud, this), wui::button_view::image_right_text, IMG_CONNECTION_CLOUD, 32, "check_button_calm"));
    localButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("credentials_dialog", "connection_local"), std::bind(&CredentialsDialog::Local, this), wui::button_view::image_right_text, IMG_CONNECTION_LOCAL, 32, "check_button_active"));
    serverText = std::shared_ptr<wui::text>(new wui::text(wui::locale("credentials_dialog", "server")));
    serverInput = std::shared_ptr<wui::input>(new wui::input((wui::config::get_int("Connection", "Secure", 0) != 0 ? "https://" : "http://") + wui::config::get_string("Connection", "Address", "server_address:8778")));
    loginText = std::shared_ptr<wui::text>(new wui::text(wui::locale("credentials_dialog", "login")));
    loginInput = std::shared_ptr<wui::input>(new wui::input(wui::config::get_string("Credentials", "Login", "")));
    passwordText = std::shared_ptr<wui::text>(new wui::text(wui::locale("credentials_dialog", "password")));
    passwordInput = std::shared_ptr<wui::input>(new wui::input(wui::config::get_string("Credentials", "Password", ""), wui::input_view::password));
    rememberCheck = std::shared_ptr<wui::button>(new wui::button(wui::locale("credentials_dialog", "remember_me"), []() {}, wui::button_view::switcher));
    rememberCheck->turn(true);
    okButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("button", "ok"), std::bind(&CredentialsDialog::OK, this)));
    cancelButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("button", "cancel"), std::bind(&CredentialsDialog::Cancel, this)));
    registrationLink = std::shared_ptr<wui::button>(new wui::button(wui::locale("credentials_dialog", "registration"), std::bind(&CredentialsDialog::Registration, this), wui::button_view::anchor));
    messageBox = std::shared_ptr<wui::message>(new wui::message(window));
    
    window->add_control(cloudButton, { 10, 50, 180, 82 });
    window->add_control(localButton, { 200, 50, 380, 82 });
    window->add_control(serverText, { 10, 100, WND_WIDTH - 10, 115 });
    window->add_control(serverInput, { 10, 120, WND_WIDTH - 10, 145 });
    window->add_control(loginText, { 10, 165, WND_WIDTH - 130, 180 });
    window->add_control(loginInput, { 10, 190, WND_WIDTH - 130, 215 });
    window->add_control(passwordText, { 10, 225, WND_WIDTH - 130, 240 });
    window->add_control(passwordInput, { 10, 250, WND_WIDTH - 130, 275 });
    window->add_control(rememberCheck, { 10, 285, 300, 323 });
    window->add_control(okButton, { WND_WIDTH - 230, WND_HEIGHT - 35, WND_WIDTH - 130, WND_HEIGHT - 10 });
    window->add_control(cancelButton, { WND_WIDTH - 110, WND_HEIGHT - 35, WND_WIDTH - 10, WND_HEIGHT - 10 });
    window->add_control(registrationLink, { 10, WND_HEIGHT - 35, 50, WND_HEIGHT - 10 });

    window->set_default_push_control(okButton);

    window->init(wui::locale("credentials_dialog", "title"), { -1, -1, WND_WIDTH, WND_HEIGHT }, wui::window_style::dialog, [this]() {
        messageBox.reset();
        registrationLink.reset();
        cancelButton.reset();
        okButton.reset();
        rememberCheck.reset();
        passwordInput.reset();
        passwordText.reset();
        loginInput.reset();
        loginText.reset();
        serverInput.reset();
        serverText.reset();
        
        if (!ok) closeCallback();
    });

    window->set_focused(loginInput);
}

void CredentialsDialog::Cloud()
{
    cloudButton->update_theme_control_name("check_button_active");
    localButton->update_theme_control_name("check_button_calm");

    serverText->hide();
    serverInput->hide();

    localServerAddress = serverInput->text();
    serverInput->set_text(CLOUD_ADDRESS);
}

void CredentialsDialog::Local()
{
    cloudButton->update_theme_control_name("check_button_calm");
    localButton->update_theme_control_name("check_button_active");

    serverText->show();
    serverInput->show();
    serverInput->set_text(localServerAddress);
}

void CredentialsDialog::OK()
{
    if (loginInput->text().empty())
    {
        return messageBox->show(wui::locale("message", "login_empty"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { window->set_focused(loginInput); });
    }

    if (rememberCheck->turned())
    {
        wui::config::set_string("Credentials", "Login", loginInput->text());
    }

    if (passwordInput->text().empty())
    {
        return messageBox->show(wui::locale("message", "password_empty"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { window->set_focused(passwordInput); });
    }

    if (rememberCheck->turned())
    {
        wui::config::set_string("Credentials", "Password", passwordInput->text());
    }

    if (CheckServerAddress())
    {
        UpdateServerAddress();
        login = loginInput->text();
        password = passwordInput->text();

        mainFrame.lock()->emit_event(4101, 0);
        
        ok = true;
        window->destroy();
    }
}

void CredentialsDialog::Cancel()
{
    ok = false;
    window->destroy();
}

void CredentialsDialog::Registration()
{
    if (CheckServerAddress())
    {
        registrationDialog = std::shared_ptr<RegistrationDialog>(new RegistrationDialog(window, std::bind(&CredentialsDialog::RegistrationEndCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
        registrationDialog->Run(serverInput->text());
    }
}

bool CredentialsDialog::CheckServerAddress()
{
    if (serverInput->text().empty())
    {
        messageBox->show(wui::locale("message", "server_url_empty"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { window->set_focused(serverInput); });

        return false;
    }

    std::vector<std::string> vals;
    boost::split(vals, serverInput->text(), boost::is_any_of("/"));
    if (vals.size() > 2)
    {
        if ((vals[0] != "http:" && vals[0] != "https:") || vals[2].empty())
        {
            messageBox->show(wui::locale("message", "incorrect_server_url"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok, [this](wui::message_result) { window->set_focused(serverInput); });

            return false;
        }
    }
    else
    {
        messageBox->show(wui::locale("message", "incorrect_server_url"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { window->set_focused(serverInput); });

        return false;
    }

    return true;
}

void CredentialsDialog::UpdateServerAddress()
{
    std::vector<std::string> vals;
    boost::split(vals, serverInput->text(), boost::is_any_of("/"));
    if (vals.size() > 2)
    {
        if (vals[0] == "http:")
        {
            wui::config::set_int("Connection", "Secure", 0);
        }
        else if (vals[0] == "https:")
        {
            wui::config::set_int("Connection", "Secure", 1);
        }
    }

    wui::config::set_string("Connection", "Address", vals[2]);
}

void CredentialsDialog::RegistrationEndCallback(RegistrationResult result, std::string_view login_, std::string_view password_)
{
    registrationDialog.reset();

    if (result == RegistrationResult::ok)
    {
        login = login_;
        password = password_;

        UpdateServerAddress();
        wui::config::set_string("Credentials", "Login", login);
        wui::config::set_string("Credentials", "Password", password);

        mainFrame.lock()->emit_event(4101, 0);

        ok = true;
        window->destroy();
    }
    else if (result == RegistrationResult::not_connection_to_server)
    {
        messageBox->show(wui::locale("common", "offline"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { window->set_focused(serverInput); });
    }
}

}
