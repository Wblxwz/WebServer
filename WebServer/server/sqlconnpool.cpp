
#include <assert.h>

#include "sqlconnpool.h"

SqlConnPool* SqlConnPool::getSqlConnPool()
{
	static SqlConnPool sqlconnpool;
	return &sqlconnpool;
}

void SqlConnPool::init(const std::string& host, const std::string& user, const std::string& pwd, const std::string& dbname, const int& port, const int& maxconn)
{
	this->host = host;
	this->user = user;
	this->pwd = pwd;
	this->dbname = dbname;
	this->port = port;

	for (int i = 0; i < maxconn; ++i)
	{
		MYSQL* con = nullptr;
		con = mysql_init(con);
		assert(con);
		con = mysql_real_connect(con, host.c_str(), user.c_str(), pwd.c_str(), dbname.c_str(), port, NULL, 0);
		assert(con);
		connVector.push_back(con);
		++freecon;
	}

	this->maxconn = maxconn;
	sem_init(&sem, 0, freecon);
}

MYSQL* SqlConnPool::getConnection()
{
	MYSQL* con = nullptr;
	if (connVector.size() == 0)
	{
		return nullptr;
	}

	//sem_wait是原子操作
	sem_wait(&sem);

	//构造自动加锁，析构自动解锁
	std::lock_guard<std::mutex> locker(mutex);

	con = connVector.front();
	connVector.erase(connVector.begin());

	--freecon;
	++curconn;

	std::cout << "getConnection" << std::endl;

	return con;
}

bool SqlConnPool::releaseConnection(MYSQL* con)
{
	if (con == nullptr)
	{
		return false;
	}

	std::lock_guard<std::mutex> locker(mutex);

	connVector.push_back(con);
	++freecon;
	--curconn;

	sem_post(&sem);

	std::cout << "releaseConnection" << std::endl;

	return true;
}

int SqlConnPool::getFreeConn()
{
	return freecon;
}

void SqlConnPool::destroyPool()
{
	std::lock_guard<std::mutex> locker(mutex);
	if (connVector.size() > 0)
	{
		for (auto& con : connVector)
		{
			mysql_close(con);
		}
		curconn = 0;
		freecon = 0;
		connVector.clear();
	}
	sem_destroy(&sem);
}