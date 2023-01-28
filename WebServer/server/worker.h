#pragma once
#include <sys/stat.h>
#include <mysql/mysql.h>
#include <map>

#include "connRAII.h"
#include "sqloperate.h"
#include "http_parse.h"

class Worker
{
public:
	Worker() = default;
	~Worker() = default;

	void init(const int& connfd,const int& epollfd,const std::string& host, const std::string& user, const std::string& pwd, const std::string& dbname, const int& port, const int& maxconn);

	Worker(const Worker&) = delete;
	Worker& operator=(const Worker&) = delete;

	int openFile(const char* filename);
	void work();
	void sendResponse(const int& cfd, const  int& fd, const int& status, const char* descr, const char* type);
	bool check(MYSQL* conn, const std::string& username, const std::string& pwd);

	//static int epollfd;
	static int usercount;
private:
	char tline[4096] = { '0' };
	std::string tstatus;
	std::string tfile;

	std::string host, user, pwd, dbname;
	std::map<const char*, const char*> content_type;

	HttpParser parser;
	struct stat st;
	SqlOperate sql;
	SqlConnPool* pool;
	Info info;

	int connfd, epollfd;
};