/**
 * Base64.h - Contains base64 helpers
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2017
 */

#pragma once

#include <string>

namespace Common
{

std::string toBase64(std::string_view source);

std::string fromBase64(std::string_view source);

}
