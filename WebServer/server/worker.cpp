#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>

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
	int fd = open(filename, O_RDONLY);
	std::cout << "filename: " << filename << std::endl;
	assert(fd >= 0);

	//零拷贝技术
	fstat(fd, &st);
	return fd;
}

void deletefd(const int& epollfd, const int& connfd)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, 0);
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
			std::cout << "errno:" << errno << " " << connfd << std::endl;
			//modfd(epollfd, connfd);
		}
		if (totle + len < sizeof(buf))
		{
			memcpy(buf + totle, temp, len);
			totle += len;
		}
	}
	modfd(epollfd, connfd);
	//读完后解析
	parser.getLine(buf, tline);
	std::cout << "tline:" << tline << std::endl;
	parser.getStatus(tline, tstatus);
	parser.getFile(tline, tfile);
	canwrite = true;
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

	if (ret <= 0)
	{
		std::cout << "errnosend:" << errno << std::endl;
	}
	off_t offset = 0;
	while (offset < st.st_size)
	{
		if (st.st_size == 0)
		{
			modfd(epollfd, connfd);
		}
		int num = sendfile(cfd, fd, &offset, st.st_size);
		if (num < 0 && errno == EAGAIN)
		{
			epoll_event e;
			e.data.fd = cfd;
			e.events = EPOLLOUT | EPOLLET | EPOLLRDHUP | EPOLLONESHOT;
			epoll_ctl(epollfd, EPOLL_CTL_MOD, cfd, &e);
			break;
		}
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
