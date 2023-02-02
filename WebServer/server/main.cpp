#include <signal.h>

#include "log.h"
#include "server.h"

int main(int argc, char* argv[])
{
	//ToDo:编写cMake
	//ToDo:考虑加密
	//ToDo:所有功能齐全后整体进行优化结构、代码，向现代C++靠拢
	//ToDo:考虑模板

	signal(SIGPIPE, SIG_IGN);

	Log::getLog()->init();
	LOG_INFO("complete init!");
	Server server(10000);
	server.init(5);
	server.serverListen();
	return 0;
}