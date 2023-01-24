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
	pool = SqlConnPool::getSqlConnPool();
	pool->init(host,user,pwd,dbname,port,maxconn);
	MYSQL* conn = nullptr;
	connRAII sqlconn(pool, conn);
	std::shared_ptr<int> s;
	
	content_type["html"] = "text/html; charset=utf-8";
	content_type["ico"] = "image/x-icon";
}

int Server::openFile(const char* filename)
{
	//ico无法send
	//注意此处没有更改tfile的值！
	filename = parser.questionMark(filename);
	fd = open(filename, O_RDONLY);
	assert(fd >= 0);

	//零拷贝技术
	fstat(fd, &st);
	return st.st_size;
}

void Server::sendResponse(const int& cfd, const int& status, const char* descr, const char* type)
{
	char buf[4096]{ '0' };

	sprintf(buf, "http/1.1 %d %s\r\n", status, descr);
	sprintf(buf + strlen(buf), "content-type: %s\r\n", type);
	sprintf(buf + strlen(buf), "content-length: %d\r\n", st.st_size);

	send(cfd, buf, strlen(buf), 0);
	std::cout << "sendResponse" << std::endl;

	sendfile(cfd, fd, NULL, st.st_size);
	std::cout << "sendFile" << std::endl;
}

void Server::serverListen()
{
	//protocol为0自动选择type类型对应的默认协议
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	//端口复用
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

		//socklen_t作为第三个参数的长度最合适
		int connfd = accept(listenfd, (sockaddr*)&caddr, &tmplen);

		int len = 0;
		char buf[4096]{ '0' };
		char temp[1024]{ '0' };

		len = recv(connfd, buf, sizeof(buf), NULL);

		int ll = 0;
		parser.getLine(buf, tline);
		std::cout << "tline:" << tline << std::endl;
		parser.getStatus(tline, tstatus);
		parser.getFile(tline, tfile);

		if (!strcmp(tstatus, "GET"))
		{
			std::string s(tfile);
			if (s == "")//没有文件请求
			{
				//-1让浏览器自行获取长度
				openFile("home.html");
				sendResponse(connfd, 200, "OK", content_type["html"]);
			}
			else
			{
				std::string s1(tfile);
				openFile(tfile);
				std::cout << "tfile:" << tfile << std::endl;
				//ToDo:favicon.ico发送不能被解析
				if (s1.find("favicon.ico") != -1)
				{
					std::cout << "123123123" << std::endl;
					sendResponse(connfd, 200, "OK", content_type["ico"]);
				}
				else
				{
					sendResponse(connfd, 200, "OK", content_type["html"]);
				}
				std::cout << tfile << std::endl;
			}
		}
		else if (!strcmp(tstatus, "POST"))
		{
			//ToDo:POST请求
			std::cout << "post tline:" << tline << std::endl;

		}

		close(connfd);
	}

	close(listenfd);
}