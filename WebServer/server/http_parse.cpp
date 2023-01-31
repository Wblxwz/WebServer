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

const int HttpParser::getStatus(const std::string& c, std::string& tstatus)
{
	//GET / HTTP/1.1
	int pos = c.find(" ");
	tstatus = c.substr(0, pos);
	return pos;
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

const int HttpParser::getFile(const std::string& tline, std::string& tfile)
{
	//GET / login.html ? HTTP / 1.1
	//std::string s(tline);
	int pos = tline.find("favicon.ico");
	if (pos == -1)
	{
		int pos1 = tline.find("/"), pos2 = tline.find("HTTP/1.1");
		if (pos2 == -1)
		{
			pos2 = tline.find("HTTP/1.0");
		}
		tfile = tline.substr(pos1 + 1, pos2 - pos1 - 2);
		return pos2 - pos1 - 3;
	}
	else
	{
		//ToDo:favicon.ico
		std::cout << "favicon.ico" << std::endl;
		tfile = tline.substr(pos, 11);
		std::cout << "ss" << tfile << std::endl;
		return 11;
	}

}

void HttpParser::getInfo(const char* buf, Info& info)
{
	std::string username(buf);
	std::string userpwd(buf);

	int pos1 = username.find("user=");
	int pos2 = username.find("&pwd=");

	username = username.substr(pos1 + 5, pos2 - (pos1 + 5));
	std::cout << "username:" << username << std::endl;

	int pos = userpwd.find("&pwd=");

	userpwd = userpwd.substr(pos + 5, userpwd.size() - (pos + 5));
	std::cout << "userpwd:" << userpwd << std::endl;

	info.setInfo(username, userpwd);
}