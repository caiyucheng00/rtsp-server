#ifndef __TIMER__H_
#define __TIMER__H_

#include <map>
#include <stdint.h>

class TimerEvent;
class EventScheduler;
class Poller;

class Timer
{
public:
	typedef uint32_t TimerId;
	typedef int64_t Timestamp; //ms
	typedef uint32_t TimeInterval; //ms

	~Timer();

	static Timestamp getCurTime();// 获取当前系统启动以来的毫秒数
	static Timestamp getCurTimestamp();// 获取毫秒级时间戳（13位）

private:
	friend class TimerManager;
	Timer(TimerEvent* event, Timestamp timestamp, TimeInterval timeInterval, TimerId timerId);

	bool handleEvent();    //TimerEvent::handleEvent()

	TimerEvent* _timerEvent;
	Timestamp _timestamp;
	TimeInterval _timeInterval;
	TimerId _timerId;

	bool _repeat;
};


class TimerManager 
{
public:
	TimerManager(EventScheduler* scheduler);
	~TimerManager();
	static TimerManager* createNew(EventScheduler* scheduler);

	Timer::TimerId addTimer(TimerEvent* event, Timer::Timestamp timestamp, Timer::TimeInterval timeInterval);
	bool removeTimer(Timer::TimerId timerId);

private:
	static void readCallback(void* arg);   // TimerManager::handleRead()
	void handleRead();

	Poller* _poller;
	std::map<Timer::TimerId, Timer> _timerMap;
	std::multimap<Timer::Timestamp, Timer> _eventMap;
	Timer::TimerId _lastTimerId;
};

#endif