#pragma once

#include <sys/stat.h>
#include <mysql/mysql.h>

#include <map>

#include "connRAII.h"
#include "http_parse.h"
#include "sqloperate.h"
#include "worker.h"
#include "threadpool.h"


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

private:
	int port, sqlport, maxconn, connfd, sqlcnt = 0;

	SqlConnPool* sqlpool;
	ThreadPool* threadpool;
};