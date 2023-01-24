#pragma once

#include <sys/stat.h>
#include <mysql/mysql.h>

#include <map>

#include "connRAII.h"
#include "http_parse.h"


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
	void sendResponse(const int& cfd, const int& status, const char* descr, const char* type);
private:
	int port, fd;

	char tline[64] = { '0' };
	char tstatus[10] = { '0' };
	char tfile[20] = { '0' };

	struct stat st;
	std::map<const char*, const char*> content_type;
	std::map<std::string, std::string> users;
	HttpParser parser;

	SqlConnPool* pool;
};