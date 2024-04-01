#ifndef __SELECTPOLLER__H_
#define __SELECTPOLLER__H_

#include "Poller.h"
#include <vector>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

class SelectPoller : public Poller
{
public:
	SelectPoller();
	virtual ~SelectPoller();
	static SelectPoller* createNew();

	virtual bool addIOEvent(IOEvent* event);
	virtual bool updateIOEvent(IOEvent* event);
	virtual bool removeIOEvent(IOEvent* event);
	virtual void handleEvent();

private:
	fd_set _readSet;
	fd_set _writeSet;
	fd_set _exceptionSet;
	int _maxNumSockets;
	std::vector<IOEvent*> _ioEvent_list;  // 存储临时活跃的IO事件对象
};

#endif