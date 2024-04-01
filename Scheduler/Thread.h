#ifndef __THREAD__H_
#define __THREAD__H_

#include <thread>

class Thread
{
public:
	Thread();
	~Thread();

	bool start(void* arg);
	bool detach();
	bool join();

	static void run(void* arg);

private:
	void* _arg;        //Threadpool对象
	bool _isStart;
	bool _isDetach;
	std::thread _thread;
};

#endif 