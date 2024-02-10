/**
 * MiniWindow.cpp - Contains mini window impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/MiniWindow.h>

#include <wui/locale/locale.hpp>
#include <wui/theme/theme.hpp>
#include <wui/system/wm_tools.hpp>

#include <resource.h>

namespace Client
{

MiniWindow::MiniWindow(std::weak_ptr<wui::window> mainWindow_)
    : mainWindow(mainWindow_),
    window(),
    cameraImage(std::make_shared<wui::image>(IMG_MW_CAMERA_ON)), microphoneImage(std::make_shared<wui::image>(IMG_MW_MICROPHONE_ON)),
    timer_(std::bind(&MiniWindow::Redraw, this)),
    cameraEnabled(false), microphoneEnabled(false),
    blinkTickCount(0)
{
}

MiniWindow::~MiniWindow()
{
}

void MiniWindow::Run()
{
    if (window)
    {
        return;
    }

    auto screenSize = wui::get_screen_size(mainWindow.lock()->context());

    window = std::shared_ptr<wui::window>(std::make_shared<wui::window>());
    window->subscribe(std::bind(&MiniWindow::ReceiveEvents, this, std::placeholders::_1), wui::event_type::mouse);

    window->add_control(cameraImage, { WND_WIDTH - 10 - microphoneImage->width() - 10 - cameraImage->width(),
        10,
        WND_WIDTH - 10 - microphoneImage->width() - 10, 10 + cameraImage->height() });

    window->add_control(microphoneImage, { WND_WIDTH - 10 - microphoneImage->width(), 10, WND_WIDTH - 10, 10 + microphoneImage->height() });

    cameraImage->show();
    microphoneImage->show();

    window->init(wui::locale("client", "title"), { screenSize.right - 10 - WND_WIDTH, 10, screenSize.right - 10, WND_HEIGHT + 10 }, wui::window_style::topmost);

    timer_.start(40);
}

void MiniWindow::End()
{
    if (window)
    {
        timer_.stop();
        window.reset();
    }
}

void MiniWindow::ShowRenderer(std::shared_ptr<wui::i_control> renderer)
{
    if (window)
    {
        mainWindow.lock()->remove_control(renderer);
        window->add_control(renderer, { 0, 0, WND_WIDTH, WND_HEIGHT });
        renderer->set_position({ 0, 0, WND_WIDTH, WND_HEIGHT });
        
        window->bring_to_front(cameraImage);
        window->bring_to_front(microphoneImage);
    }
}

void MiniWindow::SetCameraState(bool enabled)
{
    cameraEnabled = enabled;
    cameraImage->change_image(enabled ? IMG_MW_CAMERA_ON : IMG_MW_CAMERA_OFF);
}

void MiniWindow::SetMicrophoneState(bool enabled)
{
    microphoneEnabled = enabled;
    microphoneImage->change_image(enabled ? IMG_MW_MICROPHONE_ON : IMG_MW_MICROPHONE_OFF);
}

void MiniWindow::ReceiveEvents(const wui::event &ev)
{
    if (ev.type == wui::event_type::mouse)
    {
        switch (ev.mouse_event_.type)
        {
            case wui::mouse_event_type::left_up:
            case wui::mouse_event_type::right_up:
            case wui::mouse_event_type::center_up:
                mainWindow.lock()->emit_event(4050, 0);
            break;
        }
    }
}

void MiniWindow::Redraw()
{
    if (window)
    {
        window->redraw({ 0, 0, WND_WIDTH, WND_HEIGHT });
        
        if (blinkTickCount == 30)
        {
            if (cameraEnabled)
            {
                if (cameraImage->showed())
                {
                    cameraImage->hide();
                }
                else
                {
                    cameraImage->show();
                }
            }

            if (microphoneEnabled)
            {
                if (microphoneImage->showed())
                {
                    microphoneImage->hide();
                }
                else
                {
                    microphoneImage->show();
                }
            }

            blinkTickCount = 0;
        }
        ++blinkTickCount;
    }
}

}
