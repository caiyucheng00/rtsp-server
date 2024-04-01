#include "SelectPoller.h"
#include "../Utils/Log.h"

SelectPoller::SelectPoller()
{
	FD_ZERO(&_readSet);
	FD_ZERO(&_writeSet);
	FD_ZERO(&_exceptionSet);
}

SelectPoller::~SelectPoller()
{

}

SelectPoller* SelectPoller::createNew()
{
	return new SelectPoller();
}

bool SelectPoller::addIOEvent(IOEvent* event)
{
	return updateIOEvent(event);
}

bool SelectPoller::updateIOEvent(IOEvent* event)
{
	int fd = event->getFd();
	if (fd < 0) {
		LOGE("fd=%d", fd);
		return false;
	}

	FD_CLR(fd, &_readSet);
	FD_CLR(fd, &_writeSet);
	FD_CLR(fd, &_exceptionSet);

	auto it = _ioEventMap.find(fd);
	if (it != _ioEventMap.end()) {   // 已经存在
		if (event->isReadHandling()) {
			FD_SET(fd, &_readSet);
		}
		if (event->isWriteHandling()) {
			FD_SET(fd, &_writeSet);
		}
		if (event->isErrorHandling()) {
			FD_SET(fd, &_exceptionSet);
		}
	}
	else {                             // addIOEvent
		if (event->isReadHandling()) {
			FD_SET(fd, &_readSet);
		}
		if (event->isWriteHandling()) {
			FD_SET(fd, &_writeSet);
		}
		if (event->isErrorHandling()) {
			FD_SET(fd, &_exceptionSet);
		}

		_ioEventMap.insert(std::make_pair(fd, event));
	}

	if (_ioEventMap.empty()) {
		_maxNumSockets = 0;
	}
	else
	{
		_maxNumSockets = _ioEventMap.rbegin()->first + 1;   // //更新最大文件描述符+1（map会自动排序）
	}

	return true;
}

bool SelectPoller::removeIOEvent(IOEvent* event)
{
	int fd = event->getFd();
	if (fd < 0) {
		LOGE("fd=%d", fd);
		return false;
	}

	FD_CLR(fd, &_readSet);
	FD_CLR(fd, &_writeSet);
	FD_CLR(fd, &_exceptionSet);

	auto it = _ioEventMap.find(fd);
	if (it != _ioEventMap.end()) {    // 已经存在
		_ioEventMap.erase(it);
	}

	if (_ioEventMap.empty()) {
		_maxNumSockets = 0;
	}
	else
	{
		_maxNumSockets = _ioEventMap.rbegin()->first + 1;   // //更新最大文件描述符+1（map会自动排序）
	}

	return true;
}

void SelectPoller::handleEvent()
{
	fd_set readSet = _readSet;
	fd_set writeSet = _writeSet;
	fd_set exceptionSet = _exceptionSet;
	struct timeval timeout;
	int ret;
	int rEvent;

	timeout.tv_sec = 1000;
	timeout.tv_usec = 0;

	ret = select(_maxNumSockets, &readSet, &writeSet, &exceptionSet, &timeout);
	if (ret < 0) return;

	for (auto it = _ioEventMap.begin(); it != _ioEventMap.end(); ++it) {
		rEvent = 0;

		if (FD_ISSET(it->first, &readSet)) {
			rEvent |= EVENT_READ;
		}
		if (FD_ISSET(it->first, &writeSet)) {
			rEvent |= EVENT_WRITE;
		}
		if (FD_ISSET(it->first, &exceptionSet)) {
			rEvent |= EVENT_ERROR;
		}

		if (rEvent != 0) {
			it->second->setREvent(rEvent);            //控制IOEvent::handleEvent()执行
			_ioEvent_list.push_back(it->second);
		}
	}

	for (auto& ioEvent : _ioEvent_list) {
		ioEvent->handleEvent();
	}

	_ioEvent_list.clear();
}
