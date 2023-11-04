/**
 * BusyBox.cpp - Contains busy box impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/BusyBox.h>

#include <wui/locale/locale.hpp>

#include <resource.h>

namespace Client
{

static const int32_t BUSY_UPDATE = 369876;

BusyBox::BusyBox(std::weak_ptr<wui::window> transientWindow_)
    : window(new wui::window()),
    image(new wui::image(IMG_LOADING_01)),
    title(new wui::text("", wui::hori_alignment::center, wui::vert_alignment::top, "busy_title_text")),
    subTitle(new wui::text("", wui::hori_alignment::center, wui::vert_alignment::top, "busy_sub_title_text")),
    progress(new wui::progress(0, 100, 0)),
    timer_([this]() { window->emit_event(BUSY_UPDATE, 0); }),
    currentImage(0)
{
    window->set_transient_for(transientWindow_.lock());
}

BusyBox::~BusyBox()
{
}

void BusyBox::Run(std::string_view title_)
{
    title->set_text(title_);
    subTitle->set_text("");

    auto IMG_SIZE = 128;
    auto imgLeft = (WND_WIDTH - IMG_SIZE) / 2;

    window->add_control(image, { imgLeft, 20, imgLeft + IMG_SIZE, 20 + IMG_SIZE });
    window->add_control(title, { 10, WND_HEIGHT - 100, WND_WIDTH, WND_HEIGHT - 80 });
    window->add_control(subTitle, { 10, WND_HEIGHT - 70, WND_WIDTH, WND_HEIGHT - 25 });
    window->add_control(progress, { 0, WND_HEIGHT - 20, WND_WIDTH, WND_HEIGHT });

    progress->set_value(0);
    progress->hide();

    window->subscribe(std::bind(&BusyBox::UpdateImage, this, std::placeholders::_1), wui::event_type::internal);

    window->init("", { -1, -1, WND_WIDTH, WND_HEIGHT }, wui::window_style::border_all, [this]() { timer_.stop(); });

    timer_.start(333);
}

void BusyBox::SetProgress(std::string_view subTitle_, int32_t value)
{
    subTitle->set_text(subTitle_);
    progress->set_value(value);
    if (value > 0)
    {
        progress->show();
    }
}

void BusyBox::End()
{
    window->destroy();
}

void BusyBox::UpdateImage(const wui::event &ev)
{
    if (ev.internal_event_.x != BUSY_UPDATE)
    {
        return;
    }
    switch (currentImage)
    {
        case 0:
            image->change_image(IMG_LOADING_01);
        break;
        case 1:
            image->change_image(IMG_LOADING_02);
        break;
        case 2:
            image->change_image(IMG_LOADING_03);
        break;
        case 3:
            image->change_image(IMG_LOADING_04);
            currentImage = -1;
        break;
    }

    ++currentImage;
}

}
