#pragma once

//����ת��string�Ż��㷨
#include <string>
#include <iostream>

class HttpParser
{
public:
	HttpParser() {}
	~HttpParser() {}

	HttpParser(const HttpParser&) = delete;
	HttpParser& operator=(const HttpParser&) = delete;

	//���ض�ȡ�ַ��ĸ���
	const int getLine(const char* c, char* tline);
	const int getStatus(const char* c, char* tstatus);
	const int getFile(const char* tline, char* tfile);
	const int getUser(const char *tline);
	//����ʺŵĶ�ȡ����
	const char * questionMark(const char* c);
private:

};