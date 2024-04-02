#include "EventScheduler.h"
#include "SelectPoller.h"
#include "../Utils/Log.h"

EventScheduler::EventScheduler(PollerType type) :
	_quit(false)
{
#ifdef WIN32
	WSADATA wdSockMsg;//这是一个结构体
	int s = WSAStartup(MAKEWORD(2, 2), &wdSockMsg);//打开一个套接字

	if (0 != s)
	{
		switch (s)
		{
		case WSASYSNOTREADY: {
			LOGE("WSASYSNOTREADY");
			break;
		}
		case WSAVERNOTSUPPORTED: {
			LOGE("WSAVERNOTSUPPORTED");
			break;
		}
		case WSAEINPROGRESS: {
			LOGE("WSAEINPROGRESS");
			break;
		}
		case WSAEPROCLIM: {
			LOGE("WSAEPROCLIM");
			break;
		}
		}
	}

	if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
	{
		LOGE("Version Error");
		return;
	}
#endif // WIN32

	switch (type) {
		case POLLER_SELECT:
			_poller = SelectPoller::createNew();
			break;

		default:
			_exit(-1);
			break;
	}

	_timerManager = TimerManager::createNew(this);
}

EventScheduler::~EventScheduler()
{
	delete _poller;
	delete _timerManager;

#ifdef WIN32
	WSACleanup();
#endif // WIN32
}

EventScheduler* EventScheduler::createNew(PollerType type)
{
	if (type != POLLER_SELECT && type != POLLER_POLL && type != POLLER_EPOLL) {
		return NULL;
	}
		
	return new EventScheduler(type);
}

bool EventScheduler::addTriggerEvent(TriggerEvent* event)
{
	_triggerEvent_list.push_back(event);
	return true;
}

Timer::TimerId EventScheduler::addTimerEventRunAfter(TimerEvent* event, Timer::TimeInterval delay)
{
	Timer::Timestamp timestamp = Timer::getCurTime();
	timestamp += delay;

	return _timerManager->addTimer(event, timestamp, 0);
}

Timer::TimerId EventScheduler::addTimerEventRunAt(TimerEvent* event, Timer::Timestamp when)
{
	return _timerManager->addTimer(event, when, 0);
}

Timer::TimerId EventScheduler::addTimerEventRunEvery(TimerEvent* event, Timer::TimeInterval interval)
{
	Timer::Timestamp timestamp = Timer::getCurTime();
	timestamp += interval;

	return _timerManager->addTimer(event, timestamp, interval);
}

bool EventScheduler::removeTimerEvent(Timer::TimerId timerId)
{
	return _timerManager->removeTimer(timerId);
}

bool EventScheduler::addIOEvent(IOEvent* event)
{
	return _poller->addIOEvent(event);
}

bool EventScheduler::updateIOEvent(IOEvent* event)
{
	return _poller->updateIOEvent(event);
}

bool EventScheduler::removeIOEvent(IOEvent* event)
{
	return _poller->removeIOEvent(event);
}

Poller* EventScheduler::getPoller()
{
	return _poller;
}

void EventScheduler::setTimerManagerReadCallback(EventCallback callback, void* arg)
{
	_timerManagerReadCallback = callback;
	_timerManagerArg = arg;
}

void EventScheduler::loop()
{
	// 定时器
	std::thread([](EventScheduler* scheduler) {
		while (!scheduler->_quit)
		{
			if (scheduler->_timerManagerReadCallback) {
				scheduler->_timerManagerReadCallback(scheduler->_timerManagerArg);
			}
		}
		}, this).detach();

	while (!_quit) {
		handleTriggerEvents();   // 触发事件
		_poller->handleEvent();  // IO事件
	}
}

void EventScheduler::handleTriggerEvents()
{
	if (!_triggerEvent_list.empty()) {
		for (auto it = _triggerEvent_list.begin(); it != _triggerEvent_list.end(); ++it) {
			(*it)->handleEvent();
		}

		_triggerEvent_list.clear();
	}
}
