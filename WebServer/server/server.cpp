#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>

#include <iostream>
#include <memory>
#include <assert.h>

#include "server.h"

void Server::init(const int& maxconn)
{
	sqlpool = SqlConnPool::getSqlConnPool();
	threadpool = ThreadPool::getThreadPool(sqlpool);

	this->maxconn = maxconn;
}


void modfd(const int& epollfd, const int& fd, const int& ev)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
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

	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	epoll_event events;
	events.data.fd = listenfd;
	events.events = EPOLLIN;
	int n = epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &events);
	assert(n != -1);
	epoll_event eve[10000];

	while (true)
	{
		//-1即阻塞
		int num = epoll_wait(epollfd, eve, 10000, -1);
		assert(num != -1);
		for (int i = 0; i < num; ++i)
		{
			int fd = eve[i].data.fd;
			if (fd == listenfd)
			{
				sockaddr_in caddr;
				socklen_t tmplen = sizeof(caddr);

				//socklen_t作为第三个参数的长度最合适
				connfd = accept(listenfd, (sockaddr*)&caddr, &tmplen);
				assert(connfd != -1);

				int flag = fcntl(connfd, F_GETFL);
				flag |= O_NONBLOCK;
				fcntl(connfd, F_SETFL, flag);

				epoll_event even;
				even.data.fd = connfd;
				even.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
				int n = epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &even);
				assert(n != -1);
			}
			else
			{
				worker.init(eve[i].data.fd, epollfd, "localhost", "root", "a2394559659", "usersdb", 3306, 5);
				threadpool->add(&worker);
			}
		}

	}

	close(epollfd);
	close(listenfd);

}