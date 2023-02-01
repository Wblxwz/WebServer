#pragma once
#include <sys/stat.h>
#include <mysql/mysql.h>
#include <map>
#include <ctime>

#include "connRAII.h"
#include "sqloperate.h"
#include "http_parse.h"

class Worker
{
public:
	Worker() = default;
	~Worker() = default;

	void init(const int& connfd, const int& epollfd);
	time_t& getTime();
	const int& getConnfd();

	Worker(const Worker&) = delete;
	Worker& operator=(const Worker&) = delete;

	int openFile(const char* filename);
	void work();
	void sendResponse(const int& cfd, const  int& fd, const int& status, const char* descr, const char* type);
	bool check(MYSQL* conn, const std::string& username, const std::string& pwd);
	bool getIswrite();

private:
	char tline[4096] = { '0' };
	char* address;
	std::string tstatus;
	std::string tfile;
	bool iswrite = false;
	time_t timer;

	std::string host, user, pwd, dbname;
	std::map<const char*, const char*> content_type;

	HttpParser parser;
	struct stat st;
	SqlOperate sql;
	SqlConnPool* pool;
	Info info;

	int epollfd;
	int connfd;
	int fd;
};