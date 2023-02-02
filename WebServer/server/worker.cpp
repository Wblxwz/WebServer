#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include "worker.h"
#include "server.h"

void Worker::init(const int& connfd, const int& epollfd)
{
	pool = SqlConnPool::getSqlConnPool();
	this->connfd = connfd;
	this->epollfd = epollfd;

	content_type["html"] = "text/html; charset=utf-8";
	content_type["ico"] = "image/x-icon";
	content_type["jpg"] = "image/jpeg";
	content_type["rar"] = "application/octet-stream";
	content_type["mp4"] = "video/mpeg4";
}

time_t& Worker::getTime()
{
	return this->timer;
}

void modfd(const int& epollfd, const int& fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

int Worker::openFile(const char* filename)
{
	//注意此处没有更改tfile的值！
	filename = parser.questionMark(filename);
	fd = open(filename, O_RDONLY);
	assert(fd >= 0);
	//零拷贝技术
	fstat(fd, &st);
	return fd;
}

const int& Worker::getConnfd()
{
	return connfd;
}

void Worker::work()
{
	int len = 0, totle = 0;
	char buf[4096]{ '0' };
	char temp[4096]{ '0' };
	while (true)
	{
		len = recv(connfd, temp, sizeof(temp), NULL);
		if (len == -1 && errno == EAGAIN)
		{
			break;
		}
		else if (len == 0)
		{
			LOG_ERROR("Errno:%d", errno);
			LOG_ERROR("Connfd:%d", connfd);
		}
		if (totle + len < sizeof(buf))
		{
			memcpy(buf + totle, temp, len);
			totle += len;
		}
	}
	modfd(epollfd, connfd);

	LOG_INFO("Host:%s", getHost(buf));
	parser.getLine(buf, tline);
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
			if (tfile.find(".ico") != -1)
			{
				sendResponse(connfd, fd, 200, "OK", content_type["ico"]);
			}
			else if (tfile.find(".jpg") != -1)
			{
				sendResponse(connfd, fd, 200, "OK", content_type["jpg"]);
			}
			else if (tfile.find(".rar") != -1)
			{
				sendResponse(connfd, fd, 200, "OK", content_type["rar"]);
			}
			else if (tfile.find(".mp4") != -1)
			{
				sendResponse(connfd, fd, 200, "OK", content_type["mp4"]);
			}
			else
			{
				sendResponse(connfd, fd, 200, "OK", content_type["html"]);
			}
		}
	}
	else if (!strcmp(tstatus.c_str(), "POST"))
	{
		//POST请求
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
	std::string tmpline(tline);
	if (tmpline.find("HTTP/1.0") != -1)
		close(connfd);
}

const char* Worker::getHost(const char* buf)
{
	std::string hostpos(buf);
	std::string host;
	int hostpos1 = hostpos.find("Host: ");
	int hostpos2 = hostpos1 + 6;
	int cnt = 0;
	while (hostpos[hostpos2] != '\n')
	{
		host[cnt++] = hostpos[hostpos2++];
	}
	return host.c_str();
}

void Worker::sendResponse(const int& cfd, const  int& fd, const int& status, const char* descr, const char* type)
{
	char buf[4096]{ '0' };

	sprintf(buf, "http/1.1 %d %s\r\n", status, descr);
	sprintf(buf + strlen(buf), "content-type: %s\r\n", type);


	if (tfile.find("mp4") == -1)
		sprintf(buf + strlen(buf), "content-length: %d\r\n\r\n", st.st_size);
	else
	{
		sprintf(buf + strlen(buf), "content-length: %d\r\n", st.st_size);
		sprintf(buf + strlen(buf), "accept-ranges: bytes\r\n\r\n");
	}

	int ret = send(cfd, buf, strlen(buf), 0);
	//ToDo:EPOLLOUT
	off_t offset = 0;
	while (offset < st.st_size)
	{
		if (st.st_size == 0)
			modfd(epollfd, connfd);
		sendfile(connfd, fd, &offset, st.st_size);
	}
	time(&timer);
	//ToDo:多线程下关闭速度较慢
	close(fd);

	LOG_INFO("SendResponse");
}

bool Worker::check(MYSQL* conn, const std::string& username, const std::string& pwd)
{
	//暂不做更复杂的检测
	if (username.size() > 12 || pwd.size() > 16 || !sql.check(conn, username))
		return false;
	return true;
}

bool Worker::getIswrite()
{
	return iswrite;
}
