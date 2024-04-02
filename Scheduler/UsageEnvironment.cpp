#include "UsageEnvironment.h"

UsageEnvironment::UsageEnvironment(EventScheduler* scheduler, ThreadPool* threadPool) :
	_scheduler(scheduler),
	_threadPool(threadPool)
{

}

UsageEnvironment::~UsageEnvironment()
{

}

UsageEnvironment* UsageEnvironment::createNew(EventScheduler* scheduler, ThreadPool* threadPool)
{
	return new UsageEnvironment(scheduler, threadPool);
}

EventScheduler* UsageEnvironment::getScheduler()
{
	return _scheduler;
}

ThreadPool* UsageEnvironment::getThreadPool()
{
	return _threadPool;
}
