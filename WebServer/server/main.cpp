#include <signal.h>

#include "log.h"
#include "server.h"

int main(int argc, char* argv[])
{
	//ToDo:��дcMake
	//ToDo:���Ǽ���
	//ToDo:���й�����ȫ����������Ż��ṹ�����룬���ִ�C++��£
	//ToDo:����ģ��

	signal(SIGPIPE, SIG_IGN);

	Log::getLog()->init();
	LOG_INFO("complete init!");
	Server server(10000);
	server.init(5);
	server.serverListen();
	return 0;
}