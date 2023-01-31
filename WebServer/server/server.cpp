#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <time.h>

#include <iostream>
#include <memory>
#include <assert.h>

#include "server.h"

void Server::init(const int& maxconn)
{
	sqlpool = SqlConnPool::getSqlConnPool();
	sqlpool->init("localhost", "root", "a2394559659", "usersdb", 3306, 5);
	threadpool = ThreadPool::getThreadPool();

	this->maxconn = maxconn;
}

void Server::addFd(const int& epollfd, const int& fd, bool SHOT)
{
	epoll_event events;
	events.data.fd = fd;
	events.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	if (SHOT)
		events.events |= EPOLLONESHOT;
	setNoblock(fd);
	assert(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &events) != -1);
}

void Server::setNoblock(const int& fd)
{
	int flag = fcntl(fd, F_GETFL);
	assert(flag >= 0);
	flag |= O_NONBLOCK;
	assert(fcntl(fd, F_SETFL, flag) >= 0);
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
	setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

	int ret = bind(listenfd, (sockaddr*)&addr, sizeof(addr));
	assert(ret >= 0);

	ret = listen(listenfd, 5);
	assert(ret >= 0);

	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	addFd(epollfd, listenfd, false);
	epoll_event eve[10000];

	while (true)
	{
		//-1即阻塞
		int num = epoll_wait(epollfd, eve, 10000, -1);
		for (int i = 0; i < num; ++i)
		{
			int fd = eve[i].data.fd;
			if (fd == listenfd)
			{
				sockaddr_in caddr;
				socklen_t tmplen = sizeof(caddr);

				while (true)
				{
					connfd = accept(listenfd, (sockaddr*)&caddr, &tmplen);
					if (connfd < 0)
					{
						break;
					}
					addFd(epollfd, connfd, true);
				}
				continue;
			}
			else if (eve[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
			{
				printf("ERRORHUP\n");
				epoll_ctl(epollfd, EPOLL_CTL_DEL, eve[i].data.fd, 0);
				close(eve[i].data.fd);
			}
			else if (eve[i].events & EPOLLIN)
			{
				Worker *worker = new Worker;
				worker->init(eve[i].data.fd, epollfd);
				threadpool->add(worker);
			}
			else if (eve[i].events & EPOLLOUT)
			{

			}
		}
	}

	close(epollfd);
	close(listenfd);

}

