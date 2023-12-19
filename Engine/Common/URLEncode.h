/**
 * URLEncode.h - Contains url string ecnoder
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <string>

namespace Common
{

std::string URLEncode(std::string_view input) noexcept;

}
