#pragma once

//����ת��string�Ż��㷨
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

	//���ض�ȡ�ַ��ĸ���
	const int getLine(const char* c, char* tline);
	const int getStatus(const std::string& c, std::string& tstatus);
	const int getFile(const std::string& tline, std::string& tfile);
	
	void getInfo(const char* buf, Info& info);
	//����ʺŵĶ�ȡ����
	const char* questionMark(const char* c);
private:

};