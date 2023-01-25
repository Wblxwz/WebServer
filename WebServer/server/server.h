#pragma once

#include <sys/stat.h>
#include <mysql/mysql.h>

#include <map>

#include "connRAII.h"
#include "http_parse.h"
#include "sqloperate.h"


class Server
{
public:
	Server(int port) :port(port) {};
	~Server() = default;

	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	void init(const std::string& host, const std::string& user, const std::string& pwd, const std::string& dbname, const int& port, const int& maxconn);
	void serverListen();

	int openFile(const char* filename);
	void sendResponse(const int& cfd, const  int& fd, const int& status, const char* descr, const char* type);

	bool check(MYSQL* conn, const std::string& username, const std::string& pwd);
private:
	int port;

	char tline[64] = { '0' };
	std::string tstatus;
	std::string tfile;

	struct stat st;
	std::map<const char*, const char*> content_type;

	HttpParser parser;
	Info info;
	SqlOperate sql;

	std::string host, user, pwd, dbname;

	SqlConnPool* pool;
};