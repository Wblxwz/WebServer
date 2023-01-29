#pragma once

#include <sys/stat.h>
#include <mysql/mysql.h>

#include <map>

#include "connRAII.h"
#include "http_parse.h"
#include "sqloperate.h"
#include "worker.h"
#include "threadpool.h"
#include "signal.h"
#include "timer.h"

class Server
{
public:
	Server(int port) :port(port) {};
	~Server() = default;

	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	void init(const int& maxconn);
	void serverListen();
	void addFd(const int& epollfd, const int& fd,bool ONESHOT);
	void setNoblock(const int& fd);
	void timeoutHandler();
private:
	int port, sqlport,maxconn, connfd;
	int pipe[2];

	SqlConnPool* sqlpool;
	ThreadPool* threadpool;
	Worker worker;
	Signal signal;
public:
	static std::list<Timer*> timers;
};