#include "Timer.h"
#include "Event.h"
#include "EventScheduler.h"
#include "Poller.h"

Timer::Timer(TimerEvent* event, Timestamp timestamp, TimeInterval timeInterval, TimerId timerId) :
	_timerEvent(event),
	_timestamp(timestamp),
	_timeInterval(timeInterval),
	_timerId(timerId)
{
	if (timeInterval > 0) {
		_repeat = true;// 循环定时器
	}
	else {
		_repeat = false;//一次性定时器
	}
}

Timer::~Timer()
{

}

Timer::Timestamp Timer::getCurTime()
{
	long long now = std::chrono::steady_clock::now().time_since_epoch().count();
	return now / 1000000;
}

Timer::Timestamp Timer::getCurTimestamp()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

bool Timer::handleEvent()
{
	if (!_timerEvent) {
		return false;
	}

	return _timerEvent->handleEvent();
}

TimerManager::TimerManager(EventScheduler* scheduler) :
	_poller(scheduler->getPoller()),
	_lastTimerId(0)
{
	scheduler->setTimerManagerReadCallback(readCallback, this);
}

TimerManager::~TimerManager()
{
	
}

TimerManager* TimerManager::createNew(EventScheduler* scheduler)
{
	if (!scheduler) {
		return NULL;
	}

	return new TimerManager(scheduler);
}

Timer::TimerId TimerManager::addTimer(TimerEvent* event, Timer::Timestamp timestamp, Timer::TimeInterval timeInterval)
{
	++_lastTimerId;
	Timer timer(event, timestamp, timeInterval, _lastTimerId);   // 新建Timer

	_timerMap.insert(std::make_pair(_lastTimerId, timer));
	_eventMap.insert(std::make_pair(timestamp, timer));

	return _lastTimerId;
}

bool TimerManager::removeTimer(Timer::TimerId timerId)
{
	auto it = _timerMap.find(timerId);
	if (it != _timerMap.end()) {
		_timerMap.erase(timerId);
	}

	return true;
}

void TimerManager::readCallback(void* arg)
{
	TimerManager* timerManager = (TimerManager*)arg;
	timerManager->handleRead();
}

void TimerManager::handleRead()
{
	Timer::Timestamp timestamp = Timer::getCurTime();
	if (!_timerMap.empty() && !_eventMap.empty()) {
		auto it = _eventMap.begin();
		Timer timer = it->second;
		int expire = timer._timestamp - timestamp;    //时间差=过去时刻-现在时刻

		if (timestamp > timer._timestamp || expire == 0) {
			bool timerEventIsStop = timer.handleEvent();  //执行TimerEvent::handleEvent()
			_eventMap.erase(it);       // 删除（准备更新）
			if (timer._repeat) {
				if (timerEventIsStop) {
					_timerMap.erase(timer._timerId);
				}
				else
				{
					timer._timestamp = timestamp + timer._timeInterval;
					_eventMap.insert(std::make_pair(timer._timestamp, timer));  // 更新
				}
			}
			else {
				_timerMap.erase(timer._timerId);
			}
		}
	}
}
