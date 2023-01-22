

#include "server.h"

int main(int argc, char* argv[])
{
	//ToDo:优化结构、代码
	Server server(10000);
	server.init();
	server.serverListen();
	return 0;
}