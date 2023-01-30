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

std::list<Timer*> Server::timers;

void Server::init(const int& maxconn)
{
	sqlpool = SqlConnPool::getSqlConnPool();
	threadpool = ThreadPool::getThreadPool(sqlpool);

	this->maxconn = maxconn;
}

void Server::addFd(const int& epollfd, const int& fd, bool ONESHOT)
{
	epoll_event events;
	events.data.fd = fd;
	events.events = EPOLLIN | EPOLLET;
	if (ONESHOT)
		events.events |= EPOLLONESHOT;
	setNoblock(fd);
	assert(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &events) != -1);
}

void Server::setNoblock(const int& fd)
{
	int flag = fcntl(fd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flag);
}

void Server::timeoutHandler()
{
	for (auto& timer : timers)
	{
		if (timer->getTime2() - timer->getTime1() > 15)
		{
			close(timer->getSockfd());
			//timers.erase(timers.);
		}
	}
	alarm(15);
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
	epoll_event events;
	events.data.fd = listenfd;
	events.events = EPOLLIN;
	int n = epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &events);
	assert(n != -1);
	epoll_event eve[10000];

	Signal::pipefd = pipe;
	int retval = socketpair(PF_UNIX, SOCK_STREAM, 0, pipe);
	assert(retval != -1);
	//pipe[1]写端
	setNoblock(pipe[1]);
	addFd(epollfd, pipe[0], false);
	//到时信号
	signal.addSig(SIGALRM, signal.sigHandler, false);
	//kill信号
	signal.addSig(SIGTERM, signal.sigHandler, false);
	//每隔15s发送SIGALRM信号到当前进程
	alarm(15);
	bool timeout = false;
	bool stopserver = false;

	while (!stopserver)
	{
		//-1即阻塞
		int num = epoll_wait(epollfd, eve, 10000, -1);
		//errno = EINTR即阻塞被信号中断，是允许的
		if (num < 0 && errno != EINTR)
		{
			std::cout << "Error" << errno << std::endl;
			break;
		}
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
				Timer timer(connfd);
				time(&(timer.getTime1()));
				timer.rt1 = timer.getTime1();
				timers.push_back(&timer);

				addFd(epollfd, connfd, true);
			}
			else if ((fd == pipe[0]) && (eve[i].events & EPOLLIN))
			{
				int sig;
				char signals[1024];

				int rtval = recv(pipe[0], signals, sizeof(signals), 0);
				if (rtval == -1)
					continue;
				else if (rtval == 0)
					continue;
				else
				{
					for (int i = 0; i < rtval; ++i)
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
							stopserver = true;
							break;
						}
						default:
							break;
						}
					}
				}
			}
			else
			{
				if (sqlcnt == 0)
				{
					worker.init(eve[i].data.fd, epollfd, "localhost", "root", "a2394559659", "usersdb", 3306, 5);
					++sqlcnt;
				}
				else
					worker.init(eve[i].data.fd, epollfd);
				//threadpool->add(&worker);
				worker.work();
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

