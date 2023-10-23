/**
 * ActivateHandler.cpp - Contains activate window handler impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/ActivateHandler.h>

#include <Version.h>

#ifndef _WIN32
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#else
#include <tchar.h>
#endif

namespace Client
{

ActivateHandler::ActivateHandler(std::function<void()> callback_)
    : callback(callback_),
#ifdef _WIN32
    h(CreateEvent(NULL, FALSE, FALSE, _T(SYSTEM_NAME) L"RunEvent")),
#endif
    runned(true)
{
    thread = std::thread([this]() {
#ifdef _WIN32
        while (runned)
        {
            WaitForSingleObject(h, INFINITE);

            if (!runned)
            {
                break;
            }
            callback();
        }
#else
        try
        {
        	boost::interprocess::shared_memory_object smo(boost::interprocess::open_only, SYSTEM_NAME "ClientRunned", boost::interprocess::read_write);
        	boost::interprocess::mapped_region region(smo, boost::interprocess::read_write);
            while (runned)
            {
        	    auto *mem = region.get_address();

        	    if (static_cast<uint8_t*>(mem)[0] != 1)
                {
        		    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }
                else
                {
                    std::memset(mem, 0, region.get_size());

                    if (!runned)
                    {
                        break;
                    }
                    callback();
                }
            }
        }
        catch(...){}
#endif
    });
}

ActivateHandler::~ActivateHandler()
{
    runned = false;
#ifdef _WIN32
    SetEvent(h);
#endif
    if (thread.joinable()) thread.join();
#ifdef _WIN32
    CloseHandle(h);
#endif
}

}
