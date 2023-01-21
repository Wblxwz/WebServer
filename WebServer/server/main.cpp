

#include "server.h"

int main(int argc, char* argv[])
{
	Server server(10000);
	server.serverListen();
	return 0;
}