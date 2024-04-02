#ifndef __EVENTSCHEDULER__H_
#define __EVENTSCHEDULER__H_

#include <vector>
#include <queue>
#include <mutex>
#include <stdint.h>
#include "Timer.h"
#include "Event.h"

class Poller;
enum PollerType
{
	POLLER_SELECT,
	POLLER_POLL,
	POLLER_EPOLL
};

class EventScheduler
{
public:
	explicit EventScheduler(PollerType type);
	~EventScheduler();
	static EventScheduler* createNew(PollerType type);

	Poller* getPoller();
	void setTimerManagerReadCallback(EventCallback callback, void* arg);

private:
	Poller* _poller;
	bool _quit;

	EventCallback _timerManagerReadCallback;   // TimerManager::readCallback
	void* _timerManagerArg;                    // TimerManager
};

#endif