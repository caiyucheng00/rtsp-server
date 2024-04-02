#ifndef __USAGEENVIRONMENT__H_
#define __USAGEENVIRONMENT__H_

#include "ThreadPool.h"
#include "EventScheduler.h"

class UsageEnvironment
{
public:
	UsageEnvironment(EventScheduler* scheduler, ThreadPool* threadPool);
	~UsageEnvironment();
	static UsageEnvironment* createNew(EventScheduler* scheduler, ThreadPool* threadPool);

	EventScheduler* getScheduler();
	ThreadPool* getThreadPool();

private:
	EventScheduler* _scheduler;
	ThreadPool* _threadPool;
};

#endif