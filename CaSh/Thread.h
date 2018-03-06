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
		//������������һ��������Ϊ�˷���ҵ����򵥵Ľ����ǣ�
		//��ֹͬһʱ�������߳�ͬʱ����_condition,�򵥵�˵�ͺ��ź�����࣬��ֹ��ִ�еȴ�֮ǰ���Ѿ������ȴ����������
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

	//������Ч���������У�ʹ�����ڵȴ�����������ȫ���˳��ȴ���
	//�˷������ú���в�����ʹ�á�
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