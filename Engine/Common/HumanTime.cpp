/**
 * HumanTime.h - Contains human time function impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <ctime>

#include <Common/HumanTime.h>

namespace Common
{

std::string toHumanTime(time_t dt_, std::shared_ptr<wui::i_locale> locale)
{
	struct tm dt;
#ifndef _WIN32
	localtime_r(&dt_, &dt);
#else
	localtime_s(&dt, &dt_);
#endif

	std::string timeStr;

	std::string hour = (dt.tm_hour < 10 ? "0" : "") + std::to_string(dt.tm_hour);
	std::string minute = (dt.tm_min < 10 ? "0" : "") + std::to_string(dt.tm_min);
	std::string sec = (dt.tm_sec < 10 ? "0" : "") + std::to_string(dt.tm_sec);

    if (locale->get_name().find("en") != std::string::npos)
    {
        if (dt.tm_hour < 12)
        {
            if (dt.tm_hour == 0)
            {
                hour = "12";
            }
            timeStr = hour + ":" + minute + ":" + sec + " a.m.";
        }
        else
        {
            dt.tm_hour -= 12;

            if (dt.tm_hour == 0)
            {
                dt.tm_hour = 12;
            }

            hour = (dt.tm_hour < 10 ? "0" : "") + std::to_string(dt.tm_hour);
            timeStr = hour + ":" + minute + ":" + sec + " p.m.";
        }
    }
    else
    {
        timeStr = hour + ":" + minute + ":" + sec;	
	}

	auto now = time(0);
	struct tm nowDt;
#ifndef _WIN32
	localtime_r(&now, &nowDt);
#else
	localtime_s(&nowDt, &now);
#endif

	if (nowDt.tm_mday == dt.tm_mday && nowDt.tm_mon == dt.tm_mon && nowDt.tm_year == dt.tm_year)
	{
		return timeStr;
	}
	else if (nowDt.tm_mday == dt.tm_mday + 1 && nowDt.tm_mon == dt.tm_mon && nowDt.tm_year == dt.tm_year)
	{
        return locale->get("date_time", "yesterday") + " " + timeStr;
	}

	std::string month;
    switch (dt.tm_mon)
    {
        case 0: month = locale->get("date_time", "january"); break;
        case 1: month = locale->get("date_time", "february"); break;
        case 2: month = locale->get("date_time", "march"); break;
        case 3: month = locale->get("date_time", "april"); break;
        case 4: month = locale->get("date_time", "may"); break;
        case 5: month = locale->get("date_time", "june"); break;
        case 6: month = locale->get("date_time", "july"); break;
        case 7: month = locale->get("date_time", "august"); break;
        case 8: month = locale->get("date_time", "september"); break;
        case 9: month = locale->get("date_time", "october"); break;
        case 10: month = locale->get("date_time", "november"); break;
        case 11: month = locale->get("date_time", "december"); break;
    }

    if (locale->get_name().find("en") != std::string::npos)
    {
        return month + " " + std::to_string(dt.tm_mday) + (nowDt.tm_year == dt.tm_year ? "" : ", " + std::to_string(dt.tm_year + 1900)) + ", " + timeStr;
    }
    else
    {
		return std::to_string(dt.tm_mday) + " " + month + (nowDt.tm_year == dt.tm_year ? "" : " " + std::to_string(dt.tm_year + 1900)) + ", " + timeStr;
	}

	return "";
}

std::string toHumanDuration(time_t dt)
{
	if (dt < 60)
	{
		return "00:00:" + std::string(dt < 10 ? "0" : "") + std::to_string(dt);
	}
	else if (dt >= 60 && dt < 3600)
	{
		time_t minutes = dt / 60;
		time_t seconds = dt - (minutes * 60);

		return "00:" + std::string(minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
	}
	else if (dt >= 3600)
	{
		time_t hours = dt / 3600;
		time_t minutes = (dt - (hours * 3600)) / 60;
		time_t seconds = dt - (hours * 3600) - (minutes * 60);
		
		return (hours < 10 ? "0" : "") + std::to_string(hours) +
			":" + (minutes < 10 ? "0" : "") + std::to_string(minutes) + 
			":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
	}

	return "00:00:00";
}

}
