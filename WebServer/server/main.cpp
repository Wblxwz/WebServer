

#include "server.h"

int main(int argc, char* argv[])
{
	Server server(9999);
	server.serverListen();
	return 0;
}