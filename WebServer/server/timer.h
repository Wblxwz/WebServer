#pragma once

#include <netinet/in.h>
#include <time.h>

#include <list>

class Timer
{
public:
	Timer(const int& sockfd) :sockfd(sockfd) {}
	~Timer() = default;

	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;

	const int& getSockfd();
	time_t& getTime1();
	time_t& getTime2();
private:
	int sockfd;
	time_t time1,time2;
public:
	int rt1, rt2;
};