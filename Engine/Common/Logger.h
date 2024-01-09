/**
 * Logger.h - Contains common logger's initiaion methods
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#pragma once

#include <string>

namespace Common
{

std::string GetLogFileName(std::string_view appName);
void CreateLogger(std::string_view fileName);

}
