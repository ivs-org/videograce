/**
 * QuestionDialog.cpp - Contains question dialog impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/QuestionDialog.h>

#include <wui/locale/locale.hpp>

#include <resource.h>

namespace Client
{

QuestionDialog::QuestionDialog(std::weak_ptr<wui::window> transientWindow_, std::function<void(bool)> callback_)
    : missed(false),
    transientWindow(transientWindow_),
    callback(callback_),
    window(new wui::window()),
    text(),
    image(),
    yesButton(),
    noButton()
{
}

QuestionDialog::~QuestionDialog()
{
}

void QuestionDialog::Run(const std::string &question)
{
    window->set_transient_for(transientWindow.lock());
    text = std::shared_ptr<wui::text>(new wui::text());
    image = std::shared_ptr<wui::image>(new wui::image(IMG_CALLIN));
    yesButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("button", "yes"), std::bind(&QuestionDialog::Yes, this), "green_button"));
    noButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("button", "no"), std::bind(&QuestionDialog::No, this), "red_button"));

    missed = false;

    text->set_text(question);

    window->add_control(text, { 84, 10, WND_WIDTH - 10, 85 });
    window->add_control(image, { 10, 10, 74, 74 });
    window->add_control(yesButton, { WND_WIDTH - 230, WND_HEIGHT - 40, WND_WIDTH - 130, WND_HEIGHT - 10 });
    window->add_control(noButton, { WND_WIDTH - 110, WND_HEIGHT - 40, WND_WIDTH - 10, WND_HEIGHT - 10 });

    window->set_default_push_control(yesButton);

    window->init("", { -1, -1, WND_WIDTH, WND_HEIGHT }, wui::window_style::border_all, [this]() {
        noButton.reset();
        yesButton.reset();
        image.reset();
        text.reset();
    });
}

void QuestionDialog::End(bool yes)
{
    window->destroy();
    callback(yes);
}

bool QuestionDialog::IsInQuestion()
{
    return window && window->context().valid();
}

void QuestionDialog::Yes()
{
    window->destroy();
    callback(true);
}

void QuestionDialog::No()
{
    window->destroy();
    callback(false);
}

}
