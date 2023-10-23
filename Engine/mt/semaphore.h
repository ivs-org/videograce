/**
 * semaphore.h : Defines the semaphore primitive
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Official repository: https://github.com/ud84/mt
 */

#pragma once

#include <mutex>
#include <chrono>
#include <condition_variable>

namespace mt
{

class semaphore
{
public:
	semaphore()
		: mutex(), cond_var(), count(0) {}

	void notify()
	{
		std::unique_lock<std::mutex> lock(mutex);
		++count;
        cond_var.notify_one();
	}

	void wait()
	{
		std::unique_lock<std::mutex> lock(mutex);
		while (count == 0)
		{
            cond_var.wait(lock);
		}
		--count;
	}

	bool wait_for(uint32_t duration) // duration in milliseconds, return true if timeout is out
	{
		std::unique_lock<std::mutex> lock(mutex);
		while (count == 0)
		{
			if (cond_var.wait_for(lock, std::chrono::milliseconds(duration)) == std::cv_status::timeout)
			{
				return true;
			}
		}
		--count;

		return false;
	}

private:
	std::mutex mutex;
	std::condition_variable cond_var;
	uint32_t count;
};

}
