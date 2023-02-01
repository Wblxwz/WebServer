#pragma once

#include <sys/stat.h>
#include <mysql/mysql.h>

#include <map>
#include <list>

#include "connRAII.h"
#include "http_parse.h"
#include "sqloperate.h"
#include "worker.h"
#include "threadpool.h"
#include "signal.h"

class Server
{
public:
	Server(int port) :port(port) {};
	~Server() = default;

	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	void init(const int& maxconn);
	void serverListen();
	void addFd(const int& epollfd, const int& fd, bool SHOT);
	void setNoblock(const int& fd);
	void timeoutHandler();

private:
	int port, sqlport, maxconn, connfd, epollfd, sqlcnt = 0;
	time_t mytime;
	std::map<int, Worker*> workers;

	SqlConnPool* sqlpool;
	ThreadPool* threadpool;
	Signal signal;

	std::list<Worker*> Workerlist;
public:
	static int pipe[2];
private:
	void clearall(const int& connfd);
};