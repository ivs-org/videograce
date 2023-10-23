/**
 * MacAddresses.h - Contains Eternet MAC getter header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016, 2020
 */

#pragma once

#include <string>
#include <vector>

namespace Common
{
void GetMacAddresses(std::vector<std::string> &vMacAddresses);
}
