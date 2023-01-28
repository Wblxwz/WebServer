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
private:
	int port, sqlport,maxconn;

	SqlConnPool* sqlpool;
	ThreadPool* threadpool;


	int connfd;
	Worker worker;
};