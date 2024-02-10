/**
 * EnterPasswordDialog.cpp - Contains entering password dialog impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <wui/locale/locale.hpp>

#include <wui/config/config.hpp>
#include <Common/Base64.h>

#include <UI/EnterPasswordDialog.h>

#include <random>

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/nowide/convert.hpp>

#include <resource.h>

namespace Client
{

EnterPasswordDialog::EnterPasswordDialog(std::weak_ptr<wui::window> transientWindow, std::function<void(const std::string&)> readyCallback)
    : window(std::make_shared<wui::window>()),
    passwordInput(std::make_shared<wui::input>("", wui::input_view::password)),
    okButton(std::make_shared<wui::button>(wui::locale("button", "ok"), [this, readyCallback]() { window->destroy(); readyCallback(passwordInput->text()); })),
    cancelButton(std::make_shared<wui::button>(wui::locale("button", "cancel"), [this]() { window->destroy(); }))
{
    window->set_transient_for(transientWindow.lock());
}

EnterPasswordDialog::~EnterPasswordDialog()
{
}

void EnterPasswordDialog::Run()
{
    window->add_control(passwordInput, { 10, 50, WND_WIDTH - 10, 75 });
    window->add_control(okButton, { WND_WIDTH - 230, WND_HEIGHT - 40, WND_WIDTH - 130, WND_HEIGHT - 15 });
    window->add_control(cancelButton, { WND_WIDTH - 110, WND_HEIGHT - 40, WND_WIDTH - 10, WND_HEIGHT - 15 });

    window->init(wui::locale("enter_password_dialog", "title"), { -1, -1, WND_WIDTH, WND_HEIGHT }, wui::window_style::dialog);
}

}
