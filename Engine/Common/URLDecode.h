/**
 * URLDecode.h - Contains url string decoder
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>

namespace Common
{

std::string URLDecode(std::string_view input) noexcept;

}
