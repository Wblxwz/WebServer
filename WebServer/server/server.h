#pragma once

#include <sys/stat.h>
#include <mysql/mysql.h>

#include <map>

#include "connRAII.h"
#include "http_parse.h"
#include "sqloperate.h"
#include "worker.h"


class Server
{
public:
	Server(int port) :port(port) {};
	~Server() = default;

	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	void init(const std::string& host, const std::string& user, const std::string& pwd, const std::string& dbname, const int& port, const int& maxconn);
	void serverListen();
private:
	int port;

	Worker worker;
};