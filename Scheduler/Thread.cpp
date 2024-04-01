#include "Thread.h"
#include "ThreadPool.h"

Thread::Thread() :
	_arg(nullptr),
	_isStart(false),
	_isDetach(false)
{

}

Thread::~Thread()
{
	if (_isStart == true && _isDetach == false) {
		detach();
	}
}

bool Thread::start(void* arg)
{
	_arg = arg;
	_thread = std::thread(&Thread::run, _arg);

	_isStart = true;
	return _isStart;
}

bool Thread::detach()
{
	if (_isStart != true) {
		return false;
	}

	if (_isDetach == true) {
		return true;
	}

	_thread.detach();
	_isDetach = true;
	return _isDetach;
}

bool Thread::join()
{
	if (_isStart != true || _isDetach == true) {
		return false;
	}

	_thread.join();
	return true;
}

void Thread::run(void* arg)
{
	ThreadPool* threadPool = (ThreadPool*)arg;
	threadPool->loop();
}
