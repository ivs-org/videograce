/**
 * wf_queue.h : Defines the Wait-free multi-producer queue
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

template<typename T>
class waitfree_queue {
public:
	struct node {
		T data;
		node * next;
	};

	void push(const T &data)
	{
		node * n = new node;
		n->data = data;
		node * stale_head = head_.load(boost::memory_order_relaxed);
		do
		{
			n->next = stale_head;
		} while (!head_.compare_exchange_weak(stale_head, n, std::memory_order_release));
	}

	node * pop_all(void)
	{
		T * last = pop_all_reverse(), * first = 0;
		while(last)
		{
			T * tmp = last;
			last = last->next;
			tmp->next = first;
			first = tmp;
		}
		return first;
	}

	waitfree_queue() : head_(0) {}

	// alternative interface if ordering is of no importance
	node * pop_all_reverse(void)
	{
		return head_.exchange(0, std::memory_order_consume);
	}
private:
	std::atomic<node *> head_;
};

}
