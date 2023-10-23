/**
 * URI.h - Contains URI praser header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <string>

namespace Transport
{

void ParseURI(const std::string &url, std::string &proto, std::string &host, std::string &port);

}
