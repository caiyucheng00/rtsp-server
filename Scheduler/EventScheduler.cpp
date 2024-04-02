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
}

EventScheduler::~EventScheduler()
{

}

EventScheduler* EventScheduler::createNew(PollerType type)
{
	if (type != POLLER_SELECT && type != POLLER_POLL && type != POLLER_EPOLL) {
		return NULL;
	}
		
	return new EventScheduler(type);
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
