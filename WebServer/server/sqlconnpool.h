#pragma once

#include <mysql/mysql.h>
#include <deque>
#include <iostream>
#include <mutex>
#include <semaphore.h>

class SqlConnPool
{
public:
	static SqlConnPool* getSqlConnPool();

	void init(const std::string& host, const std::string& user, const std::string& pwd, const std::string& dbname, const int& port, const int& maxconn);
	MYSQL* getConnection();
	bool releaseConnection(MYSQL* c);
	int getFreeConn();
	void destroyPool();

	SqlConnPool(const SqlConnPool&) = delete;
	SqlConnPool& operator=(const SqlConnPool&) = delete;
private:
	SqlConnPool() {}
	~SqlConnPool() 
	{
		destroyPool();
	}

	sem_t sem;
	std::mutex mutex;

	std::deque<MYSQL*> connDeque;
	std::string host, user, pwd, dbname;
	int port, maxconn, curconn = 0, freecon = 0;
};