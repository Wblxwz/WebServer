#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <string.h>

#include "worker.h"

void Worker::setconnfd(const int& connfd)
{
	this->connfd = connfd;
}

void Worker::init(const std::string& host, const std::string& user, const std::string& pwd, const std::string& dbname, const int& port, const int& maxconn)
{
	pool = SqlConnPool::getSqlConnPool();
	pool->init(host, user, pwd, dbname, port, maxconn);

	content_type["html"] = "text/html; charset=utf-8";
	content_type["ico"] = "image/x-icon";
	content_type["jpg"] = "image/jpeg";
}

int Worker::openFile(const char* filename)
{
	//注意此处没有更改tfile的值！
	filename = parser.questionMark(filename);
	int fd = open(filename, O_RDONLY);
	assert(fd >= 0);

	//零拷贝技术
	fstat(fd, &st);
	return fd;
}

void Worker::work()
{
	int len = 0;
	char buf[4096]{ '0' };
	char temp[1024]{ '0' };

	len = recv(connfd, buf, sizeof(buf), NULL);

	int ll = 0;
	parser.getLine(buf, tline);
	std::cout << "tline:" << tline << std::endl;
	parser.getStatus(tline, tstatus);
	parser.getFile(tline, tfile);

	if (!strcmp(tstatus.c_str(), "GET"))
	{
		int fd = 0;
		if (tfile == "")//没有文件请求
		{
			//-1让浏览器自行获取长度
			fd = openFile("home.html");
			sendResponse(connfd, fd, 200, "OK", content_type["html"]);
		}
		else
		{
			fd = openFile(tfile.c_str());
			std::cout << "tfile:" << tfile << std::endl;
			//ToDo:favicon.ico发送不能被解析
			if (tfile.find(".ico") != -1)
			{
				std::cout << "123123123" << std::endl;
				sendResponse(connfd, fd, 200, "OK", content_type["ico"]);
			}
			else if (tfile.find(".jfif") != -1)
			{
				sendResponse(connfd, fd, 200, "OK", content_type["jpg"]);
			}
			else
			{
				sendResponse(connfd, fd, 200, "OK", content_type["html"]);
			}
			std::cout << tfile << std::endl;
		}
	}
	else if (!strcmp(tstatus.c_str(), "POST"))
	{
		//POST请求
		std::cout << "post tline:" << tline << std::endl;
		MYSQL* conn = nullptr;
		connRAII sqlconn(pool, &conn);
		parser.getInfo(buf, info);
		sql.useDb(conn);
		int fd = 0;
		if (tfile == "signup")
		{
			std::string username = info.getInfo().first;
			std::string pwd = info.getInfo().second;
			if (check(conn, username, pwd))
			{
				fd = openFile("home.html");
				sql.insert(conn, username, pwd);
				sendResponse(connfd, fd, 200, "OK", content_type["html"]);
			}
			else
			{
				fd = openFile("signuperror.html");
				sendResponse(connfd, fd, 200, "OK", content_type["html"]);
			}
		}
		else
		{
			//login
			std::string pwd = sql.search(conn, info.getInfo().first);
			if (pwd.empty())
			{
				fd = openFile("loginerror.html");
				sendResponse(connfd, fd, 200, "OK", content_type["html"]);
			}
			else
			{
				if (pwd == info.getInfo().second)
				{
					fd = openFile("loginsuccess.html");
					sendResponse(connfd, fd, 200, "OK", content_type["html"]);
				}
				else
				{
					fd = openFile("loginerror.html");
					sendResponse(connfd, fd, 200, "OK", content_type["html"]);
				}
			}
		}
	}
	close(connfd);
}

void Worker::sendResponse(const int& cfd, const  int& fd, const int& status, const char* descr, const char* type)
{
	char buf[4096]{ '0' };
	int num = 0;

	num = sprintf(buf, "http/1.1 %d %s\r\n", status, descr);
	num = sprintf(buf + num, "content-type: %s\r\n", type);
	sprintf(buf + num + 4, "content-length: %d\r\n\r\n", st.st_size);

	int ret = send(cfd, buf, strlen(buf), 0);
	assert(ret > 0);

	off_t offset = 0;
	while (offset < st.st_size)
	{
		std::cout << "num:" << sendfile(cfd, fd, &offset, st.st_size) << std::endl;
	}
	close(fd);
	std::cout << "sendResponse" << std::endl;
}

bool Worker::check(MYSQL* conn, const std::string& username, const std::string& pwd)
{
	//暂不做更复杂的检测
	if (username.size() > 12 || pwd.size() > 16 || !sql.check(conn, username))
		return false;
	return true;
}