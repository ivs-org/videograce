/**
 * MainToolBar.cpp - Contains main toolbar impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <wui/locale/locale.hpp>
#include <wui/theme/theme.hpp>

#include <UI/TimerBar.h>

namespace Client
{

std::shared_ptr<wui::i_theme> MakeTimeTextTheme(bool back)
{
    auto timeTextTheme = wui::make_custom_theme();

    timeTextTheme->load_theme(*wui::get_default_theme());

    if (back)
    {
        auto color = timeTextTheme->get_color(wui::text::tc, wui::text::tv_color);
        timeTextTheme->set_color(wui::text::tc, wui::text::tv_color, 0xFFFFFF - color);
    }

    auto font = timeTextTheme->get_font(wui::text::tc, wui::text::tv_font);
    font.size = 36;
    timeTextTheme->set_font(wui::text::tc, wui::text::tv_font, font);

    return timeTextTheme;
}

TimerBar::TimerBar(std::weak_ptr<wui::window> mainFrame_, Common::TimeMeter &timeMeter_, std::function<void()> limitEndCallback_)
    : parent(mainFrame_),
    timeMeter(timeMeter_),
    limitEndCallback(limitEndCallback_),
    timer_(std::bind(&TimerBar::OnTimer, this)),
    timeTextFront(new wui::text()), timeTextBack(new wui::text()),
    backTime(0), limitTime(0), estimateTime(0)
{
    parent.lock()->add_control(timeTextBack, { 0 });
    parent.lock()->add_control(timeTextFront, { 0 });

    timeTextFront->update_theme(MakeTimeTextTheme(false));
    timeTextFront->set_topmost(true);
    timeTextFront->hide();
    timeTextBack->update_theme(MakeTimeTextTheme(true));
    timeTextBack->set_topmost(true);
    timeTextBack->hide();
}

TimerBar::~TimerBar()
{
}

void TimerBar::Run(time_t limit, int32_t left, int32_t top)
{
    limitTime = limit;

    auto parent_ = parent.lock();
    if (parent_)
    {
        left += 10;
        top += 10;

        timeTextBack->set_position({ left + SHIFT + 1, top + SHIFT + 1, left + SHIFT + TEXT_WIDTH + 1, top + HEIGHT - SHIFT + 1 });
        timeTextBack->show();
        timeTextFront->set_position({ left + SHIFT, top + SHIFT, left + SHIFT + TEXT_WIDTH, top + HEIGHT - SHIFT });
        timeTextFront->show();

        timer_.start();
    }
}

void TimerBar::End()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        timer_.stop();

        //parent_->remove_control(timeTextBack);
        //parent_->remove_control(timeTextFront);

        timeTextBack->hide();
        timeTextFront->hide();
    }
}

void TimerBar::UpdatePosition(int32_t left, int32_t top)
{
    left += 10;
    top += 10;

    timeTextBack->set_position({ left + SHIFT + 1, top + SHIFT + 1, left + SHIFT + TEXT_WIDTH + 1, top + HEIGHT - SHIFT + 1 });
    timeTextFront->set_position({ left + SHIFT, top + SHIFT, left + SHIFT + TEXT_WIDTH, top + HEIGHT - SHIFT });
}

void TimerBar::UpdateTheme()
{
    timeTextFront->update_theme(MakeTimeTextTheme(false));
    timeTextBack->update_theme(MakeTimeTextTheme(true));
}

void TimerBar::OnTimer()
{
    backTime = timeMeter.Measure() / 1000000;

    struct tm backTm = { 0 };
#ifndef _WIN32
    gmtime_r(&backTime, &backTm);
#else
    gmtime_s(&backTm, &backTime);
#endif

    std::string timeStr = (backTm.tm_hour < 10 ? "0" : "") + std::to_string(backTm.tm_hour) + ":" +
        (backTm.tm_min < 10 ? "0" : "") + std::to_string(backTm.tm_min) + ":" +
        (backTm.tm_sec < 10 ? "0" : "") + std::to_string(backTm.tm_sec);

    if (limitTime != 0)
    {
        estimateTime = limitTime - backTime;

        struct tm estimateTm = { 0 };
#ifndef _WIN32
        gmtime_r(&estimateTime, &estimateTm);
#else
        gmtime_s(&estimateTm, &estimateTime);
#endif

        std::string estimate = (estimateTm.tm_hour < 10 ? "0" : "") + std::to_string(estimateTm.tm_hour) + ":" +
            (estimateTm.tm_min < 10 ? "0" : "") + std::to_string(estimateTm.tm_min) +
            ":" + (estimateTm.tm_sec < 10 ? "0" : "") + std::to_string(estimateTm.tm_sec);

        timeStr += " / " + estimate;

        if (limitTime <= backTime)
        {
            limitTime = 0;
            limitEndCallback();
        }
    }

    timeTextBack->set_text(timeStr);
    timeTextFront->set_text(timeStr);
}

}
