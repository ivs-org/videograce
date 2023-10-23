/**
 * HumanTime.h - Contains human time function header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <memory>
#include <wui/locale/i_locale.hpp>

namespace Common
{

std::string toHumanTime(time_t dt, std::shared_ptr<wui::i_locale> locale);
std::string toHumanDuration(time_t dt);

}
