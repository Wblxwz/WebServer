#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <signal.h>

#include <iostream>
#include <memory>
#include <assert.h>

#include "server.h"

void Server::init(const std::string& host, const std::string& user, const std::string& pwd, const std::string& dbname, const int& port, const int& maxconn)
{
	worker.init(host, user, pwd, dbname, port, maxconn);
}

void Server::serverListen()
{
	//protocolΪ0�Զ�ѡ��type���Ͷ�Ӧ��Ĭ��Э��
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	//�˿ڸ���
	int tmp = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp));

	int ret = bind(listenfd, (sockaddr*)&addr, sizeof(addr));
	assert(ret >= 0);

	ret = listen(listenfd, 5);
	assert(ret >= 0);

	while (true)
	{
		sockaddr_in caddr;
		socklen_t tmplen = sizeof(caddr);

		//socklen_t��Ϊ�����������ĳ��������
		int connfd = accept(listenfd, (sockaddr*)&caddr, &tmplen);

		worker.setconnfd(connfd);
		worker.work();
	}
	close(listenfd);
}