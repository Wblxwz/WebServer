
#include "threadpool.h"

ThreadPool::ThreadPool(SqlConnPool* sqlpool, const int& threadnum = 1, const int& maxrequestsnum = 100) :sqlconnpool(sqlpool), threadnum(threadnum), maxrequestsnum(maxrequestsnum), threads(nullptr)
{
	sem_init(&sem, 0, 0);
	assert(threadnum > 0 && maxrequestsnum > 0);

	threads = new pthread_t[threadnum];
	assert(threads);

	for (int i = 0; i < threadnum; ++i)
	{
		if (pthread_create(threads + i, NULL, worker, this) != 0)
		{
			delete[] threads;
			abort();
		}
		if (pthread_detach(threads[i]) != 0)
		{
			delete threads;
			abort();
		}
	}
}

ThreadPool* ThreadPool::getThreadPool(SqlConnPool* sqlpool, const int& threadnum, const int& maxrequestsnum)
{
	static ThreadPool pool(sqlpool, threadnum, maxrequestsnum);
	return &pool;
}

bool ThreadPool::add(Worker* worker)
{
	std::unique_lock<std::mutex> locker(mutex);

	if (workerdeque.size() > maxrequestsnum)
	{
		locker.unlock();
		return false;
	}
	
	workerdeque.push_back(worker);
	locker.unlock();

	//信号量加一，即有任务需要处理
	sem_post(&sem);

	return true;
}

void ThreadPool::run()
{
	while (true)
	{
		sem_wait(&sem);
		std::unique_lock<std::mutex> locker(mutex);


		if (workerdeque.empty())
		{
			locker.unlock();
			continue;
		}

		Worker* request = workerdeque.front();
		workerdeque.pop_front();
		locker.unlock();

		if (!request)
			continue;

		std::cout << "work: " << request->connfd << std::endl;
		request->work();
	}
}

void* ThreadPool::worker(void* arg)
{
	ThreadPool* pool = (ThreadPool*)arg;
	pool->run();
	return pool;
}

ThreadPool::~ThreadPool()
{
	std::lock_guard<std::mutex> locker(mutex);

	if (!workerdeque.empty())
	{
		for (auto& i : workerdeque)
		{
			delete i;
		}
	}

	sem_destroy(&sem);
	delete[]threads;
}