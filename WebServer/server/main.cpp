

#include "server.h"

int main(int argc, char* argv[])
{
	//ToDo:�Ż��ṹ������
	Server server(10000);
	server.init();
	server.serverListen();
	return 0;
}