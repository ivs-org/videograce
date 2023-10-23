/**
 * FSHelpers.h - Contains helpers for file system manipulation
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>

namespace Common
{

bool CheckAllowFileWrite(const std::string &path);
std::string DirNameOf(const std::string& fileNameWithPath);
std::string FileNameOf(const std::string& fileNameWithPath);

}
