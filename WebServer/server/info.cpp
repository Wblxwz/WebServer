

#include "info.h"

void Info::setInfo(const std::string& username, const std::string& pwd)
{
	this->username = username;
	this->pwd = pwd;
}

std::pair<std::string, std::string> Info::getInfo()
{
	std::pair<std::string, std::string> p(username, pwd);
	return p;
}