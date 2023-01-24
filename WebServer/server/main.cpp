

#include "server.h"

int main(int argc, char* argv[])
{
	//ToDo:考虑加密
	//ToDo:优化结构、代码
	Server server(10000);
	server.init("localhost", "root", "a2394559659", "usersdb", 3306, 5);
	server.serverListen();
	return 0;
}