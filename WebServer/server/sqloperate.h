#pragma once
#include <mysql/mysql.h>


#include <iostream>
#include <assert.h>

class SqlOperate
{
public:
	SqlOperate() {}
	~SqlOperate() {}

	SqlOperate(const SqlOperate&) = delete;
	SqlOperate& operator=(const SqlOperate&) = delete;

	void useDb(MYSQL* conn);
	void insert(MYSQL *conn,const std::string& username,const std::string& userpwd);
	std::string search(MYSQL* conn, const std::string& username);
private:

};