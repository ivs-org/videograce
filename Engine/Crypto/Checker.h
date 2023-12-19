/**
 * Checker.h - Contains ssl checker interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <string>

namespace Crypto
{

bool CheckSSLCertificate(std::string_view certificate_, std::string_view privateKey_);

}
