#pragma once

#include <deque>
#include <mutex>
#include <semaphore.h>
#include <pthread.h>

#include "worker.h"
#include "sqlconnpool.h"

class ThreadPool
{
public:
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	bool add(Worker* worker);

	static ThreadPool* getThreadPool(SqlConnPool* sqlpool, const int& threadnum = 1, const int& maxrequestsnum = 100);
private:
	ThreadPool(SqlConnPool* sqlpool, const int& threadnum, const int& maxrequestsnum);
	~ThreadPool();

	static void* worker(void *arg);
	void run();


	int threadnum, maxrequestsnum;

	pthread_t* threads;

	std::mutex mutex;
	sem_t sem;

	std::deque<Worker*> workerdeque;
	SqlConnPool* sqlconnpool;
};