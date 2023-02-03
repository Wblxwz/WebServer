#pragma once
class Signal
{
public:
	Signal() {}
	~Signal() = default;

	Signal(const Signal&) = delete;
	Signal& operator=(const Signal&) = delete;

	//�źŴ�����
	static void sigHandler(int sig);
	//�����źź���
	void addSig(const int& sig, void (*handler)(int), bool restart = true);
};

