/**
 * DialingDialog.cpp - Contains dialing dialog impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/DialingDialog.h>

#include <wui/locale/locale.hpp>

#include <resource.h>

namespace Client
{

DialingDialog::DialingDialog(std::weak_ptr<wui::window> transientWindow_, std::function<void()> cancelCallback_)
    : transientWindow(transientWindow_),
    cancelCallback(cancelCallback_),
    window(new wui::window()),
    text(),
    image(),
    cancelButton()
{
}

DialingDialog::~DialingDialog()
{
}

void DialingDialog::Run(std::string_view subscriber)
{
    window->set_transient_for(transientWindow.lock());
    text = std::shared_ptr<wui::text>(new wui::text());
    image = std::shared_ptr<wui::image>(new wui::image(IMG_CALLOUT));
    cancelButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("button", "cancel"), std::bind(&DialingDialog::Cancel, this)));

    text->set_text(wui::locale("dialing_dialog", "dial") + " " + std::string(subscriber));

    window->add_control(text, { 84, 10, WND_WIDTH - 10, 65 });
    window->add_control(image, { 10, 10, 74, 74 });
    window->add_control(cancelButton, { WND_WIDTH - 110, WND_HEIGHT - 40, WND_WIDTH - 10, WND_HEIGHT - 10 });

    window->set_default_push_control(cancelButton);

    window->init("", { -1, -1, WND_WIDTH, WND_HEIGHT }, wui::window_style::border_all, [this]() {
        cancelButton.reset();
        image.reset();
        text.reset();
    });
}

void DialingDialog::End()
{
    if (window)
    {
        window->destroy();
    }
}

void DialingDialog::Cancel()
{
    if (window)
    {
        window->destroy();
    }
    cancelCallback();
}

}
