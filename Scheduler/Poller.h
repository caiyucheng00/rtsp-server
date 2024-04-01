#ifndef __POLLER__H_
#define __POLLER__H_

#include <map>
#include "Event.h"

class Poller
{
public:
	virtual ~Poller();

	virtual bool addIOEvent(IOEvent* event) = 0;
	virtual bool updateIOEvent(IOEvent* event) = 0;
	virtual bool removeIOEvent(IOEvent* event) = 0;
	virtual void handleEvent() = 0;

protected:
	Poller();

	std::map<int, IOEvent*> _ioEventMap;
};

#endif