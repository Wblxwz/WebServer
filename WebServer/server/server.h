#pragma once

class Server
{
public:
	Server(int port) :port(port) {};
	~Server() = default;

	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	void serverListen();
	void getLine(const char* c, const int& len);

	int sendFile(const char* filename, const int& cfd);
	void sendResponse(const int& cfd, const int& status, const char* descr, const char* type, const int& len);
private:
	int port;
	char tbuf[1024];
};