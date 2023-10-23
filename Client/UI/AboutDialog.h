/**
 * AboutDialog.h - Contains about dialog header
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

class AboutDialog
{
public:
    AboutDialog(std::weak_ptr<wui::window> transientWindow);
    ~AboutDialog();

    void Run(uint32_t grants);

private:
    static const int32_t WND_WIDTH = 400, WND_HEIGHT = 600;

    std::weak_ptr<wui::window> transientWindow;

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::image> logo;
    std::shared_ptr<wui::text> appName, client;
    std::shared_ptr<wui::text> version;
    std::shared_ptr<wui::text> license;

    std::shared_ptr<wui::text> wuiInfo;
    std::shared_ptr<wui::button> wuiURLAnchor;

    std::shared_ptr<wui::button> vendorURLAnchor;
    std::shared_ptr<wui::text> copyright;
    std::shared_ptr<wui::button> closeButton;
};

}
