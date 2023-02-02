#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <assert.h>

#include "log.h"

template<class T>
class BlockQueue
{
public:
	BlockQueue(const int& max = 1000)
	{
		assert(max > 0);
		this->max = max;
	}
	~BlockQueue() = default;

	BlockQueue(const BlockQueue&) = delete;
	BlockQueue& operator=(const BlockQueue&) = delete;
	bool push(T elem);
	bool pop(T& elem);
private:
	std::condition_variable producer, consumer;
	std::mutex mutex;
	std::deque<T> deque;
	int max;
};

template<class T>
inline bool BlockQueue<T>::push(T elem)
{
	std::unique_lock<std::mutex> locker(mutex);
	while (deque.size() >= max)
	{
		producer.wait(locker);
	}
	deque.push_back(elem);
	consumer.notify_one();
	return true;
}

template<class T>
inline bool BlockQueue<T>::pop(T& elem)
{
	std::unique_lock<std::mutex> locker(mutex);
	while (deque.empty())
	{
		consumer.wait(locker);
	}
	elem = deque.front();
	deque.pop_front();
	producer.notify_one();
	return true;
}