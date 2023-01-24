#pragma once

//考虑转成string优化算法
#include <string>
#include <iostream>

#include "info.h"

class HttpParser
{
public:
	HttpParser() {}
	~HttpParser() {}

	HttpParser(const HttpParser&) = delete;
	HttpParser& operator=(const HttpParser&) = delete;

	//返回读取字符的个数
	const int getLine(const char* c, char* tline);
	const int getStatus(const std::string& c, std::string& tstatus);
	const int getFile(const std::string& tline, std::string& tfile);
	
	void getInfo(const char* buf, Info& info);
	//解决问号的读取问题
	const char* questionMark(const char* c);
private:

};