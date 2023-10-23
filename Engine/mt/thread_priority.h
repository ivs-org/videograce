/**
 * thread_priority.h : Defines the helper change thread priority
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Official repository: https://github.com/ud84/mt
 */

#pragma once

#include <thread>

#ifndef _WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif

namespace mt
{

enum class priority_type
{
	low,
	normal,
	higher,
	real_time
};

static int set_thread_priority(std::thread &th, priority_type tp)
{
#ifndef _WIN32
	if (tp >= priority_type::higher)
	{
		int policy = 0;
		struct sched_param param = { 0 };
		pthread_getschedparam(th.native_handle(), &policy, &param);
		param.sched_priority = sched_get_priority_max(policy);
		return pthread_setschedparam(th.native_handle(), policy, &param);
	}
#else
	int pm = THREAD_PRIORITY_NORMAL;
	switch (tp)
	{
        case priority_type::low:
			pm = THREAD_PRIORITY_LOWEST;
		break;
        case priority_type::higher:
			pm = THREAD_PRIORITY_HIGHEST;
		break;
        case priority_type::real_time:
			pm = THREAD_PRIORITY_TIME_CRITICAL;
		break;
		default:
		break;
	}

	return SetThreadPriority(th.native_handle(), pm);
#endif

	return 0;
}

}
