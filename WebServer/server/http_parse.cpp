#include <string.h>

#include "http_parse.h"

const int HttpParser::getLine(const char* c, char* tline)
{
	int i = 0;
	while (c[i] != '\n')
	{
		tline[i] = c[i];
		++i;
	}
	tline[i] = '\0';
	return i;
}

const int HttpParser::getStatus(const char* c, char* tstatus)
{
	//GET / HTTP/1.1
	std::string s(c);
	int pos = s.find(" ");
	int cnt = 0;
	s = s.substr(0, pos);
	for (auto i : s)
	{
		tstatus[cnt++] = i;
	}
	tstatus[cnt] = '\0';
	return cnt;
}

const char* HttpParser::questionMark(const char* c)
{
	std::string s(c);
	if (s.find("?") != -1)
	{
		s = s.substr(0, s.length() - 1);
		return s.c_str();
	}
	return s.c_str();
}

const int HttpParser::getFile(const char* tline, char* tfile)
{
	//GET / login.html ? HTTP / 1.1
	std::string s(tline);
	int pos = s.find("favicon.ico");
	if (pos == -1)
	{
		int pos1 = s.find("/"), pos2 = s.find("HTTP/1.1");
		std::string ss = s.substr(pos1 + 1, pos2 - pos1 - 2);
		//POST / login HTTP / 1.1
		int cnt = 0;
		for (auto i : ss)
		{
			tfile[cnt++] = i;
		}
		tfile[cnt] = '\0';
		return pos2 - pos1 - 3;
	}
	else
	{
		//ToDo:favicon.ico
		std::cout << "favicon.ico" << std::endl;
		int cnt = 0;
		std::string ss = s.substr(pos, 11);
		for (auto i : ss)
		{
			tfile[cnt] = i;
			++cnt;
		}
		tfile[cnt] = '\0';
		std::cout << "ss" << tfile << std::endl;
		return 11;
	}

}

const int HttpParser::getUser(const char* tline)
{

}