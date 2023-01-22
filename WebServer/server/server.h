#pragma once

#include <map>

#include "http_parse.h"


class Server
{
public:
	Server(int port) :port(port) {};
	~Server() = default;

	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	void init();

	void serverListen();

	int sendFile(const char* filename, const int& cfd);
	void sendResponseHead(const int& cfd, const int& status, const char* descr, const char* type, const int& len);
private:
	int port;
	char tline[128] = { '0' };
	char tstatus[10] = { '0' };
	char tfile[20] = { '0' };
	std::map<const char*, const char*> content_type;
	HttpParser parser;
};