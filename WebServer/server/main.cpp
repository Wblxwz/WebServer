

#include "server.h"

int main(int argc, char* argv[])
{
	//ToDo:���Ǽ���
	//ToDo:���й�����ȫ����������Ż��ṹ�����룬���ִ�C++��£
	//ToDo:����ģ��
	Server server(10000);
	server.init("localhost", "root", "a2394559659", "usersdb", 3306, 5);
	server.serverListen();
	return 0;
}