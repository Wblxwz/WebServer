#include <string.h>

#include "sqloperate.h"

void SqlOperate::useDb(MYSQL* conn)
{
	std::string sql = "USE usersdb;";
	int ret = mysql_real_query(conn, sql.c_str(), sql.length());
	assert(ret == 0);
}

void SqlOperate::insert(MYSQL* conn, const std::string& username, const std::string& userpwd)
{
	char sql[100] = { '0' };
	sprintf(sql, "INSERT INTO users VALUES ('%s', '%s')", username.c_str(), userpwd.c_str());
	int ret = mysql_real_query(conn, sql, strlen(sql));
	assert(ret == 0);
}

std::string SqlOperate::search(MYSQL* conn, const std::string& username)
{
	char sql[100] = { '0' };
	sprintf(sql, "SELECT pwd FROM users WHERE name = '%s'", username.c_str());
	int ret = mysql_real_query(conn, sql, strlen(sql));
	assert(ret == 0);
	MYSQL_RES* res = nullptr;
	res = mysql_store_result(conn);
	MYSQL_ROW row = nullptr;
	while ((row = mysql_fetch_row(res)))
	{
		std::cout << row[0] << std::endl;
		return row[0];
	}
	return mysql_error(conn);
}