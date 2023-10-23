/**
 * TimerBar.h - Contains main timer bar header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/panel.hpp>
#include <wui/control/text.hpp>

#include <mt/timer.h>
#include <Common/TimeMeter.h>

namespace Client
{

class TimerBar
{
public:
    TimerBar(std::weak_ptr<wui::window> mainFrame, Common::TimeMeter &timeMeter, std::function<void()> limitEndCallback);
    ~TimerBar();

    void Run(time_t limit, int32_t left, int32_t top);
    void End();

    void UpdatePosition(int32_t left, int32_t top);

    void UpdateTheme();

private:
    static const int32_t HEIGHT = 32, SHIFT = 5, TEXT_WIDTH = 220;
    
    std::weak_ptr<wui::window> parent;
    Common::TimeMeter &timeMeter;
    std::function<void()> limitEndCallback;

    mt::timer timer_;

    std::shared_ptr<wui::text> timeTextFront, timeTextBack;

    time_t backTime, limitTime, estimateTime;

    void OnTimer();
};

}
