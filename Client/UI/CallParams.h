/**
 * CallParams.h : Defines params related to the call
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <string>
#include <cstdint>

namespace Client
{

struct CallParams
{
    std::string callingSubscriber;
    int64_t subscriberId;
    int32_t subscriberConnectionId;
    std::string subscriberName;

    std::string scheduleConferenceTag, scheduleConferenceName;

    time_t timeLimit;

    CallParams()
        : callingSubscriber(),
        subscriberId(-1),
        subscriberConnectionId(-1),
        subscriberName(),

        scheduleConferenceTag(),
        scheduleConferenceName(),

        timeLimit(0)
    {}

    void Clear()
    {
        callingSubscriber.clear();
        subscriberId = -1;
        subscriberConnectionId =-1;
        subscriberName.clear();

        scheduleConferenceTag.clear();

        timeLimit = 0;
    }
};

}
