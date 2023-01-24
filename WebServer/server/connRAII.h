#pragma once
#include <mysql/mysql.h>

#include "sqlconnpool.h"

class connRAII
{
public:
	connRAII(SqlConnPool* pool, MYSQL** con) :pool(pool)
	{
		*con = pool->getConnection();
		conn = *con;
	}
	~connRAII()
	{
		pool->releaseConnection(conn);
	}
private:
	SqlConnPool* pool;
	MYSQL* conn;
};