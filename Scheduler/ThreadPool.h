#ifndef __THREADPOOL__H_
#define __THREADPOOL__H_

#include <queue>
#include <vector>
#include <mutex> 
#include <condition_variable>
#include "Thread.h"

class Task {
public:
	typedef void (*TaskCallback)(void*);

	Task();

	void setTaskCallback(TaskCallback callback, void* arg);
	void handle();

	bool operator=(const Task& task);

private:
	TaskCallback _taskCallback;
	void* _arg;
};

class ThreadPool
{
public:
	explicit ThreadPool(int num);
	~ThreadPool();
	static ThreadPool* createNew(int num);

	void addTask(Task& task);
	void loop();

private:
	void createThreads();
	void cancelThreads();

	std::vector<Thread> _thread_list;
	std::queue<Task> _task_queue;
	std::mutex _mutex;
	std::condition_variable _cv;
	bool _quit;
};

#endif