#include "Event.h"
#include "../Utils/Log.h"

TriggerEvent::TriggerEvent(void* arg) :
	_arg(arg),
	_triggerCallback(nullptr)
{
	LOGI("TriggerEvent()");
}

TriggerEvent::~TriggerEvent()
{
	LOGI("~TriggerEvent()");
}

TriggerEvent* TriggerEvent::createNew(void* arg)
{
	return new TriggerEvent(arg);
}

TriggerEvent* TriggerEvent::createNew()
{
	return new TriggerEvent(NULL);
}

void TriggerEvent::setArg(void* arg)
{
	_arg = arg;
}

void TriggerEvent::setTriggerCallback(EventCallback callback)
{
	_triggerCallback = callback;
}

void TriggerEvent::handleEvent()
{
	if (_triggerCallback) {
		_triggerCallback(_arg);
	}
}

TimerEvent::TimerEvent(void* arg) :
	_arg(arg),
	_timeoutCallback(nullptr),
	_isStop(false)
{
	LOGI("TimerEvent()");
}

TimerEvent::~TimerEvent()
{
	LOGI("~TimerEvent()");
}

TimerEvent* TimerEvent::createNew(void* arg)
{
	return new TimerEvent(arg);
}
TimerEvent* TimerEvent::createNew()
{
	return new TimerEvent(NULL);
}

void TimerEvent::setArg(void* arg)
{
	_arg = arg;
}

void TimerEvent::setTimeoutCallback(EventCallback callback)
{
	_timeoutCallback = callback;
}

bool TimerEvent::handleEvent()
{
	if (_isStop) {
		return _isStop;
	}

	if (_timeoutCallback) {
		_timeoutCallback(_arg);
	}

	return _isStop;
}

void TimerEvent::stop()
{
	_isStop = true;
}

IOEvent::IOEvent(int fd, void* arg) :
	_fd(fd),
	_arg(arg),
	_event(EVENT_NONE),
	_revent(EVENT_NONE),
	_readCallback(NULL),
	_writeCallback(NULL),
	_errorCallback(NULL)
{
	LOGI("IOEvent() fd=%d", _fd);
}

IOEvent::~IOEvent()
{
	LOGI("~IOEvent() fd=%d", _fd);
}

IOEvent* IOEvent::createNew(int fd, void* arg)
{
	if (fd < 0)
		return NULL;

	return new IOEvent(fd, arg);
}

IOEvent* IOEvent::createNew(int fd)
{
	if (fd < 0)
		return NULL;

	return new IOEvent(fd, NULL);
}

int IOEvent::getFd() const
{
	return _fd;
}

int IOEvent::getEvent() const
{
	return _event;
}

void IOEvent::setREvent(int event)
{
	_revent = event;
}

void IOEvent::setArg(void* arg)
{
	_arg = arg;
}

void IOEvent::setReadCallback(EventCallback callback)
{
	_readCallback = callback;
}

void IOEvent::setWriteCallback(EventCallback callback)
{
	_writeCallback = callback;
}

void IOEvent::setErrorCallback(EventCallback callback)
{
	_errorCallback = callback;
}

void IOEvent::enableReadHandling()
{
	_event |= EVENT_READ;
}

void IOEvent::enableWriteHandling()
{
	_event |= EVENT_WRITE;
}

void IOEvent::enableErrorHandling()
{
	_event |= EVENT_ERROR;
}

void IOEvent::disableReadeHandling()
{
	_event &= ~EVENT_READ;
}

void IOEvent::disableWriteHandling()
{
	_event &= ~EVENT_WRITE;
}

void IOEvent::disableErrorHandling()
{
	_event &= ~EVENT_ERROR;
}

bool IOEvent::isNoneHandling() const
{
	return _event == EVENT_NONE;
}

bool IOEvent::isReadHandling() const
{
	return (_event & EVENT_READ) != 0;
}

bool IOEvent::isWriteHandling() const
{
	return (_event & EVENT_WRITE) != 0;
}

bool IOEvent::isErrorHandling() const
{
	return (_event & EVENT_ERROR) != 0;
}

void IOEvent::handleEvent()
{
	if (_readCallback && (_revent & EVENT_READ)) {
		_readCallback(_arg);
	}

	if (_writeCallback && (_revent & EVENT_WRITE)) {
		_writeCallback(_arg);
	}

	if (_errorCallback && (_revent & EVENT_ERROR)) {
		_errorCallback(_arg);
	}
}
