#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>
#include <thread>
#include <memory>
#include "RBMathUtilities.h"
#include <future>

template <class T>
class RBThreadSafeQueue
{
public:
  RBThreadSafeQueue()
  {
    _valid = true;
  }
	~RBThreadSafeQueue()
	{
		invalidate();
	}

	bool try_pop(T& out)
	{
		std::lock_guard<std::mutex> lock{ _mutex };
		if (_queue.empty() || _valid)
			return false;
		out = std::move(_queue.front());
		_queue.pop();
		return true;
	}

	bool wait_pop(T& out)
	{
		std::lock_guard<std::mutex> lock{ _mutex };
		//条件变量加上一把锁，是为了分离业务，最简单的解释是：
		//防止同一时刻两个线程同时操作_condition,简单地说就和信号量差不多，防止在执行等待之前就已经出发等待造成了死锁
		_condition.wait(lock, [this](){return !_queue.empty() || !_valid; });
		if (_valid)
			return false;
		out = std::move(_queue.front());
		_queue.pop();
		return true;
	}

	void pop_all(std::vector<T>& outv)
	{
		std::lock_guard<std::mutex> lock{ _mutex };
		T& t = _queue.front();
		outv.push_back(t);
		_queue.pop();
	}

	void push(T value)
	{
		std::lock_guard<std::mutex> lock{ _mutex };
		_queue.push(std::move(value));
		_condition.notify_one();
	}

	bool empty(void) const
	{
		std::lock_guard<std::mutex> lock{ _mutex };
		return _queue.empty();
	}

	void clear()
	{
		std::lock_guard<std::mutex> lock{ _mutex };
		while (!_queue.empty())
		{
			_queue.pop();
		}
		_condition.notify_all();
	}

	//用于无效化整个队列，使得正在等待的条件变量全部退出等待。
	//此方法调用后队列不能再使用。
	void invalidate()
	{
		std::lock_guard<std::mutex> lock{ _mutex };
		_valid = false;
		_condition.notify_all();
	}

	bool is_valid() const
	{
		std::lock_guard<std::mutex> lock{ _mutex };
		return _valid;
	}


private:
	std::atomic_bool _valid;
	mutable std::mutex _mutex;
	std::queue<T> _queue;
	std::condition_variable _condition;
};