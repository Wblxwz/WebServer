#pragma once
#include <map>

class Info
{
public:
	Info() = default;
	~Info() = default;

	void setInfo(const std::string& username, const std::string& pwd);

	std::pair<std::string, std::string> getInfo();

	Info(const Info&) = delete;
	Info& operator=(const Info&) = delete;
private:
	std::string username;
	std::string pwd;
};