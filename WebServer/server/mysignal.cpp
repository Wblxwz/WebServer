#include <errno.h>
#include <memory.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <assert.h>

#include "mysignal.h"
#include "server.h"


//�����źŴ�����ֻ��ͨ���ܵ������ź�ֵ�������첽ִ��ʱ��
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
	//ʹ���źŴ�ϵ�ϵͳ�����Զ����·���
	if (restart)
		sa.sa_flags |= SA_RESTART;
	//�źŴ�������ִ���ڼ����������ź�
	sigfillset(&sa.sa_mask);

	assert(sigaction(sig, &sa, nullptr) != -1);
}