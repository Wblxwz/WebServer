#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <signal.h>

#include <iostream>
#include <assert.h>

#include "server.h"

void Server::init()
{
	content_type["html"] = "text/html; charset=utf-8";
	content_type["ico"] = "image/x-icon";
}

int Server::sendFile(const char* filename, const int& cfd)
{
	//ico无法open
	//注意此处没有更改tfile的值！
	filename = parser.questionMark(filename);
	int fd = open(filename, O_RDONLY);
	assert(fd >= 0);
	while (true)
	{
		char buf[1024];
		int len = read(fd, buf, sizeof(buf));
		if (len > 0)
		{
			int ret = send(cfd, buf, len, MSG_NOSIGNAL);
			/*if (ret == -1)
			{
				if (errno != EAGAIN)
				{
					break;
				}
			}*/
			usleep(10);
		}
		else if (len == 0)
		{
			break;
		}
		else
		{
			abort();
		}
	}
	std::cout << "sendFile" << std::endl;
	return 0;
}

void Server::sendResponseHead(const int& cfd, const int& status, const char* descr, const char* type, const int& len)
{
	char buf[4096]{ '0' };
	sprintf(buf, "http/1.1 %d %s\r\n", status, descr);
	sprintf(buf + strlen(buf), "content-type: %s\r\n", type);
	sprintf(buf + strlen(buf), "content-length: %d\r\n", len);
	send(cfd, buf, strlen(buf), 0);
	std::cout << "sendResponse" << std::endl;
}

void Server::serverListen()
{
	//protocol为0自动选择type类型对应的默认协议
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	//端口复用
	int tmp = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp));

	int ret = bind(listenfd, (sockaddr*)&addr, sizeof(addr));
	assert(ret >= 0);

	ret = listen(listenfd, 5);
	assert(ret >= 0);

	while (true)
	{
		sockaddr_in caddr;
		socklen_t tmplen = sizeof(caddr);

		//socklen_t作为第三个参数的长度最合适
		int connfd = accept(listenfd, (sockaddr*)&caddr, &tmplen);

		int len = 0;
		char buf[4096]{ '0' };
		char temp[1024]{ '0' };

		len = recv(connfd, buf, sizeof(buf), NULL);

		int ll = 0;
		parser.getLine(buf, tline);
		std::cout << "tline:" << tline << std::endl;
		parser.getStatus(tline, tstatus);
		parser.getFile(tline, tfile);

		if (!strcmp(tstatus, "GET"))
		{
			std::string s(tfile);
			if (s == "0")//没有文件请求
			{
				//-1让浏览器自行获取长度
				sendResponseHead(connfd, 200, "OK", content_type["html"], -1);
				sendFile("home.html", connfd);
			}
			else
			{
				std::string s1(tfile);
				if (s1.find("favicon.ico") != -1)
				{
					std::cout << "123123123" << std::endl;
					sendResponseHead(connfd, 200, "OK", content_type["ico"], -1);
				}
				else
				{
					sendResponseHead(connfd, 200, "OK", content_type["html"], -1);
				}
				sendFile(tfile, connfd);
				std::cout << tfile << std::endl;
				std::cout << "tline:" << tline << std::endl;
			}
		}
		else if (!strcmp(tstatus, "POST"))
		{
			//ToDo:POST请求
			std::cout << "tfile:" << tline << std::endl;
		}

		close(connfd);
	}

	close(listenfd);
}