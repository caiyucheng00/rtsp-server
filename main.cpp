#include "Scheduler/EventScheduler.h"
#include "Scheduler/ThreadPool.h"
#include "Scheduler/UsageEnvironment.h"


int main() {
	srand(time(NULL));

	EventScheduler* scheduler = EventScheduler::createNew(POLLER_SELECT);
	ThreadPool* threadPool = ThreadPool::createNew(2);
	UsageEnvironment* env = UsageEnvironment::createNew(scheduler, threadPool);
	
	env->getScheduler()->loop();

	return 0;
}