#pragma once

//考虑转成string优化算法
#include <string>
#include <iostream>

class HttpParser
{
public:
	HttpParser() {}
	~HttpParser() {}

	HttpParser(const HttpParser&) = delete;
	HttpParser& operator=(const HttpParser&) = delete;

	//返回读取字符的个数
	const int getLine(const char* c, char* tline);
	const int getStatus(const char* c, char* tstatus);
	const int getFile(const char* tline, char* tfile);
	const int getUser(const char *tline);
	//解决问号的读取问题
	const char * questionMark(const char* c);
private:

};