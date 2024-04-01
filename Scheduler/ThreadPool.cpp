#include "ThreadPool.h"

Task::Task() :
	_taskCallback(nullptr),
	_arg(nullptr)
{

}

void Task::setTaskCallback(TaskCallback callback, void* arg)
{
	_taskCallback = callback;
	_arg = arg;
}

void Task::handle()
{
	if (_taskCallback) {
		_taskCallback(_arg);
	}
}

bool Task::operator=(const Task& task)
{
	this->_taskCallback = task._taskCallback;
	this->_arg = task._arg;
	return true;
}

ThreadPool::ThreadPool(int num) :
	_thread_list(num),
	_quit(false)
{
	createThreads();   //thread.start()---Thread::run---loop()【多个loop】
}

ThreadPool::~ThreadPool()
{

}

ThreadPool* ThreadPool::createNew(int num)
{
	return new ThreadPool(num);
}

void ThreadPool::addTask(Task& task)
{
	std::unique_lock <std::mutex> lck(_mutex);
	_task_queue.push(task);
	_cv.notify_one();
}

void ThreadPool::loop()
{
	printf("loop start\n");
	while (!_quit)
	{
		std::unique_lock <std::mutex> lck(_mutex);
		if (_task_queue.empty()) {
			_cv.wait(lck);            // 任务队列没有任务，阻塞等待
		}
		// 任务队列有任务, 唤醒
		if (_task_queue.empty()) {
			continue;
		}

		Task task = _task_queue.front();
		_task_queue.pop();

		task.handle();         // 处理任务
	}
	printf("loop end\n");
}

void ThreadPool::createThreads()
{
	std::unique_lock <std::mutex> lck(_mutex);
	for (auto& thread : _thread_list) {
		thread.start(this);
	}
}

void ThreadPool::cancelThreads()
{
	std::unique_lock <std::mutex> lck(_mutex);
	_quit = true;
	_cv.notify_all();

	for (auto& thread : _thread_list) {
		thread.join();
	}
	_thread_list.clear();
}
