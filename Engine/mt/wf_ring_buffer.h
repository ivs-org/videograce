/**
 * wf_ring_buffer.h : Defines the wait free ring buffer
 *
 * Author: https://www.boost.org/doc/libs/1_54_0/doc/html/atomic/usage_examples.html
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Official repository: https://github.com/ud84/mt
 */

#pragma once

#include <atomic>

namespace mt
{

template<typename T, size_t Size>
class ringbuffer
{
public:
	ringbuffer() : head_(0), tail_(0) {}

	bool push(const T & value)
	{
		size_t head = head_.load(std::memory_order_relaxed);
		size_t next_head = next(head);
		if (next_head == tail_.load(std::memory_order_acquire))
			return false;
		ring_[head] = value;

		head_.store(next_head, std::memory_order_release);

		return true;
	}

	bool pop(T & value)
	{
		size_t tail = tail_.load(std::memory_order_relaxed);
		if (tail == head_.load(std::memory_order_acquire))
			return false;

		value = ring_[tail];
		tail_.store(next(tail), std::memory_order_release);

		return true;
	}

private:
	size_t next(size_t current)
	{
		return (current + 1) % Size;
	}

	T ring_[Size];
	std::atomic<size_t> head_, tail_;
};

}
