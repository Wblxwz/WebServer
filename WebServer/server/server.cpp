#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

#include <iostream>
#include <assert.h>

#include "server.h"

void Server::getLine(const char* c, const int& len)
{
	int i = 0;
	while (i < len && c[i] != '\n')
	{
		tbuf[i] = c[i];
		++i;
	}
}

int Server::sendFile(const char* filename, const int& cfd)
{
	int fd = open(filename, O_RDONLY);
	assert(fd >= 0);
	while (true)
	{
		char buf[1024];
		int len = read(fd, buf, sizeof(buf));
		std::cout << len << std::endl;
		if (len > 0)
		{
			send(cfd, buf, len, 0);
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

void Server::sendResponse(const int& cfd, const int& status, const char* descr, const char* type, const int& len)
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

		getLine(buf, len);
		//std::cout << tbuf << std::endl;
		char bufg[20]{ '0' };
		for (int i = 0; i < 3; ++i)
		{
			bufg[i] = tbuf[i];
		}
		//std::cout << bufg << std::endl;
		if (!strcmp(bufg, "GET"))
		{
			//-1让浏览器自行获取长度
			sendResponse(connfd, 200, "OK", "text/html; charset=utf-8", -1);
			sendFile("./home.html", connfd);
		}
		close(connfd);
	}

	close(listenfd);
}