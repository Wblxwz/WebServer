#include <errno.h>
#include <memory.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <assert.h>

#include "mysignal.h"
#include "server.h"


//不做信号处理，只是通过管道发送信号值，减少异步执行时间
void Signal::sigHandler(int sig)
{
	int tmperrno = errno;
	int msg = sig;

	assert(send(Server::pipe[1], (char*)&msg, 1, 0) != -1);
	errno = tmperrno;
}

void Signal::addSig(const int& sig, void(*handler)(int), bool restart)
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));

	sa.sa_handler = handler;
	//使被信号打断的系统调用自动重新发起
	if (restart)
		sa.sa_flags |= SA_RESTART;
	//信号处理函数执行期间屏蔽所有信号
	sigfillset(&sa.sa_mask);

	assert(sigaction(sig, &sa, nullptr) != -1);
}
