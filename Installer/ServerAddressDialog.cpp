/**
 * ServerAddressDialog.cpp - Contains enter the server address dialog impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <boost/algorithm/string.hpp>
#include <boost/nowide/convert.hpp>

#include <ServerAddressDialog.h>

namespace Installer
{

ServerAddressDialog::ServerAddressDialog()
    : window(new wui::window()),
    input(new wui::input()),
    okButton(new wui::button(wui::locale("button", "ok"), std::bind(&ServerAddressDialog::OnOK, this))),
    cancelButton(new wui::button(wui::locale("button", "cancel"), [this]() { window->destroy(); })),
    messageBox(new wui::message(window)),
    ok(false)
{
}

ServerAddressDialog::~ServerAddressDialog()
{
}

void ServerAddressDialog::Run(std::shared_ptr<wui::window> transientWindow, std::function<void(bool ok)> result_callback)
{
    bool secure = wui::config::get_int("Connection", "Secure", 0) != 0;
    std::string server = wui::config::get_string("Connection", "Address", "");

    input->set_text((secure != 0 ? "https://" : "http://") + server);

    window->add_control(input, { 10, 55, WND_WIDTH - 10, 80 });
    window->add_control(okButton, { WND_WIDTH - 230, 100, WND_WIDTH - 130, 125 });
    window->add_control(cancelButton, { WND_WIDTH - 110, 100, WND_WIDTH - 10, 125 });

    window->set_transient_for(transientWindow);

    window->init(wui::locale("installer", "server_address"), { -1, -1, WND_WIDTH, WND_HEIGHT },
        wui::window_style::dialog, [result_callback, this]() { result_callback(ok); });
}

void ServerAddressDialog::ShowIncorrectURLError()
{
    messageBox->show(wui::locale("installer_error", "incorrect_server_url"), wui::locale("installer_error", "title"), wui::message_icon::alert, wui::message_button::ok, [](wui::message_result) {});
}

void ServerAddressDialog::OnOK()
{
    std::string serverFull = input->text();

    if (serverFull.empty())
    {
        messageBox->show(wui::locale("installer_error", "empty_server_url"), wui::locale("installer_error", "title"), wui::message_icon::alert, wui::message_button::ok, [](wui::message_result) {});
        return;
    }

    bool secureComm = false;

    std::vector<std::string> vals;
    boost::split(vals, serverFull, boost::is_any_of("/"));
    if (vals.size() > 2)
    {
        if (vals[0] == "http:")
        {
            secureComm = false;
        }
        else if (vals[0] == "https:")
        {
            secureComm = true;
        }
        else
        {
            ShowIncorrectURLError();
            return;
        }
    }
    else
    {
        ShowIncorrectURLError();
        return;
    }

    wui::config::set_int("Connection", "Secure", secureComm ? 1 : 0);
    wui::config::set_string("Connection", "Address", vals[2]);

    ok = true;
    window->destroy();
}

}
