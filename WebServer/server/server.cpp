#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

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
		std::cout << tbuf << std::endl;

		close(connfd);
	}

	close(listenfd);
}