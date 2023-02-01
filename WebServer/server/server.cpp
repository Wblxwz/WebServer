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

int Server::pipe[];

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

void Server::clearall(const int& connfd)
{
	std::cout << "close:" << connfd << "  " << errno << std::endl;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, 0);
	auto it = workers.find(connfd);
	delete it->second;
	workers.erase(it);
	close(connfd);
}

void Server::timeoutHandler()
{
	time(&mytime);
	for (auto i = workers.cbegin(); i != workers.cend(); ++i)
	{
		if (mytime - i->second->getTime() > 15)
		{
			int confd = i->first;
			clearall(confd);
		}
	}
	//alarm只触发一次
	alarm(5);
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

	epollfd = epoll_create(5);
	assert(epollfd != -1);
	addFd(epollfd, listenfd, false);
	epoll_event eve[10000];

	int retval = socketpair(PF_UNIX, SOCK_STREAM, 0, pipe);
	setNoblock(pipe[1]);
	addFd(epollfd, pipe[0], false);
	signal.addSig(SIGALRM, Signal::sigHandler, false);
	signal.addSig(SIGTERM, Signal::sigHandler, false);

	bool stop = false;
	bool timeout = false;

	alarm(5);

	while (!stop)
	{
		//-1即阻塞
		int num = epoll_wait(epollfd, eve, 10000, -1);
		if (num < 0 && errno != EINTR)
			break;
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
					Worker* worker = new Worker;
					worker->init(connfd, epollfd);
					time(&(worker->getTime()));
					workers[connfd] = worker;
				}
				continue;
			}
			else if (eve[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
			{
				printf("ERRORHUP\n");
				clearall(eve[i].data.fd);
			}
			else if ((eve[i].data.fd == pipe[0]) && (eve[i].events & EPOLLIN))
			{
				int sig;
				char signals[1024];
				int ret = recv(pipe[0], signals, sizeof(signals), 0);
				if (ret == -1)
				{
					std::cout << "pipeError" << errno << std::endl;
					continue;
				}
				else if (ret == 0)
					continue;
				else
				{
					for (int i = 0; i < ret; ++i)
					{
						switch (signals[i])
						{
						case SIGALRM:
						{
							timeout = true;
							break;
						}
						case SIGTERM:
						{
							stop = true;
							break;
						}
						default:
							break;
						}
					}
				}
			}
			else if (eve[i].events & EPOLLIN)
			{
				Worker* worker = workers[eve[i].data.fd];
				Workerlist.push_back(worker);
				threadpool->add(worker);
			}
			else if (eve[i].events & EPOLLOUT)
			{
				std::cout << "EPOLLOUT" << std::endl;
				/*Worker* worker = workers[eve[i].data.fd];
				threadpool->add(worker);*/
			}
		}
		if (timeout)
		{
			timeoutHandler();
			timeout = false;
		}
	}

	close(epollfd);
	close(listenfd);

}

