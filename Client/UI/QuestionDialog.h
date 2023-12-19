/**
 * QuestionDialog.h - Contains question dialog header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/text.hpp>
#include <wui/control/image.hpp>
#include <wui/control/button.hpp>

#include <string>
#include <memory>
#include <functional>

namespace Client
{

class QuestionDialog
{
public:
    QuestionDialog(std::weak_ptr<wui::window> transientWindow, std::function<void(bool)> callback);
    ~QuestionDialog();

    void Run(std::string_view question);
    void End(bool yes);

    bool IsInQuestion();

    bool missed;

private:
    static const int32_t WND_WIDTH = 300, WND_HEIGHT = 150;

    std::weak_ptr<wui::window> transientWindow;
    std::function<void(bool)> callback;

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::text> text;
    std::shared_ptr<wui::image> image;
    std::shared_ptr<wui::button> yesButton, noButton;
    
    void Yes();
    void No();
};

}
