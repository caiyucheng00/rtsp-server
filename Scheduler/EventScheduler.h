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

	bool addTriggerEvent(TriggerEvent* event);
	Timer::TimerId addTimerEventRunAfter(TimerEvent* event, Timer::TimeInterval delay);
	Timer::TimerId addTimerEventRunAt(TimerEvent* event, Timer::Timestamp when);
	Timer::TimerId addTimerEventRunEvery(TimerEvent* event, Timer::TimeInterval interval);
	bool removeTimerEvent(Timer::TimerId timerId);
	bool addIOEvent(IOEvent* event);
	bool updateIOEvent(IOEvent* event);
	bool removeIOEvent(IOEvent* event);

	Poller* getPoller();
	void setTimerManagerReadCallback(EventCallback callback, void* arg);

	void loop();

private:
	void handleTriggerEvents();

	Poller* _poller;
	bool _quit;
	TimerManager* _timerManager;

	std::vector<TriggerEvent*> _triggerEvent_list;
	std::mutex _mutex;

	EventCallback _timerManagerReadCallback;   // TimerManager::readCallback
	void* _timerManagerArg;                    // TimerManager
};

#endif