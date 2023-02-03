#include <pthread.h>
#include <string.h>
#include <stdarg.h>

#include "log.h"

Log* Log::getLog()
{
	static Log log;
	return &log;
}

void Log::init(const int& log_buf_size, const int& split_lines, const int& max_queue_size)
{
	srand(time(0));
	queue = new BlockQueue<std::string>(max_queue_size);
	pthread_t tid;
	pthread_create(&tid, NULL, Log::threadWrite, NULL);
	this->log_buf_size = log_buf_size;
	this->split_lines = split_lines;

	time_t t = time(nullptr);
	now = localtime(&t);
	sprintf(filename, "log/%d_%02d_%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
	fp = fopen(filename, "a");
	assert(fp != nullptr);
}

void* Log::threadWrite(void* args)
{
	Log::getLog()->asyncWriteLog();
}

void Log::asyncWriteLog()
{
	std::string s;
	while (queue->pop(s))
	{
		std::lock_guard<std::mutex> locker(mutex);
		fputs(s.c_str(), fp);
		fflush(fp);
	}
}

void Log::writeLog(const int& level, const char* format, ...)
{
	char s[32] = { 0 };
	std::unique_lock<std::mutex> locker(mutex);
	sprintf(s, "%02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec);
	switch (level)
	{
	case 0:
		sprintf(s + strlen(s), "[debug]:");
		break;
	case 1:
		sprintf(s + strlen(s), "[info]:");
		break;
	case 2:
		sprintf(s + strlen(s), "[warn]:");
		break;
	case 3:
		sprintf(s + strlen(s), "[error]:");
		break;
	default:
		sprintf(s + sizeof(s), "[info]:");
		break;
	}
	++count;
	int year = now->tm_year + 1900, month = now->tm_mon + 1, day = now->tm_mday;
	time_t tt = time(nullptr);
	now = localtime(&tt);
	if ((year != (now->tm_year + 1900)) || (month != (now->tm_mon + 1)) || (day != now->tm_mday) || count >= split_lines)
	{
		fflush(fp);
		fclose(fp);
		char filename[256];
		sprintf(filename, "log/%d_%02d_%02d_%d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, rand() % 65535);
		fp = fopen(filename, "a");
		year = now->tm_year + 1900;
		month = now->tm_mon + 1;
		day = now->tm_mday;
		count = 0;
	}
	locker.unlock();
	va_list va;
	va_start(va, format);
	std::string logstr;
	char tbuf[8192];
	locker.lock();
	int n = sprintf(tbuf, "%s", s);
	int m = vsnprintf(tbuf + n, log_buf_size - 1, format, va);
	tbuf[n + m] = '\n';
	tbuf[n + m + 1] = '\0';
	logstr = tbuf;
	locker.unlock();
	queue->push(logstr);
	va_end(va);
}
