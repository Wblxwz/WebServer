#pragma once
class Signal
{
public:
	Signal() {}
	~Signal() = default;

	Signal(const Signal&) = delete;
	Signal& operator=(const Signal&) = delete;

	//信号处理函数
	static void sigHandler(int sig);
	//设置信号函数
	void addSig(const int& sig, void (*handler)(int), bool restart = true);
};

