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
private:
	int port;
	char tbuf[1024];
};