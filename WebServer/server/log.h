#pragma once
#include <time.h>

#include "blockqueue.h"

class Log
{
public:
	static Log* getLog();

	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;

	void init(const int& log_buf_size = 8192, const int& split_lines = 5000000, const int& max_queue_size = 1000);
	static void* threadWrite(void* args);
	void asyncWriteLog();
	void writeLog(const int& level, const char* format, ...);
private:
	Log() = default;
	~Log() = default;

	int log_buf_size, split_lines;
	char filename[256];
	int count = 0;
	tm* now;
	std::string buf;
	std::mutex mutex;
	FILE* fp;
	BlockQueue<std::string>* queue;
};

#define LOG_DEBUG(format, ...) Log::getLog()->writeLog(0, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) Log::getLog()->writeLog(1, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) Log::getLog()->writeLog(2, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Log::getLog()->writeLog(3, format, ##__VA_ARGS__)