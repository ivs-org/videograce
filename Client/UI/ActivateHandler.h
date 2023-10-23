/**
 * ActivateHandler.h - Contains activate window handler header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <thread>

#include <functional>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Client
{

class ActivateHandler
{
	
public:
    ActivateHandler(std::function<void()> callback);
    ~ActivateHandler();

private:
    std::function<void()> callback;
#ifdef _WIN32
    HANDLE h;
#endif
    bool runned;
    std::thread thread;

};

}
