/**
 * Qouter.h - Contains quoter helper
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>

inline std::string quot(std::string_view in)
{
	return "\"" + std::string(in) + "\"";
}
