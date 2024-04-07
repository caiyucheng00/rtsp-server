#ifndef __EVENT__H_
#define __EVENT__H_

typedef void (*EventCallback)(void*);

class TriggerEvent
{
public:
	TriggerEvent(void* arg);
	~TriggerEvent();
	static TriggerEvent* createNew(void* arg);
	static TriggerEvent* createNew();

	void setArg(void* arg);
	void setTriggerCallback(EventCallback callback);
	void handleEvent();

private:
	void* _arg;
	EventCallback _triggerCallback;
};

class TimerEvent 
{
public:
	TimerEvent(void* arg);
	~TimerEvent();
	static TimerEvent* createNew(void* arg);
	static TimerEvent* createNew();

	void setArg(void* arg);
	void setTimeoutCallback(EventCallback callback);
	bool handleEvent();
	void stop();

private:
	void* _arg;    // Sink
	EventCallback _timeoutCallback;  //Sink::handleTimeout()
	bool _isStop;
};

enum IOEventType
{
	EVENT_NONE = 0,
	EVENT_READ = 1,
	EVENT_WRITE = 2,
	EVENT_ERROR = 4,
};

class IOEvent 
{
public:
	IOEvent(int fd, void* arg);
	~IOEvent();
	static IOEvent* createNew(int fd, void* arg);
	static IOEvent* createNew(int fd);

	int getFd() const;
	int getEvent() const;
	void setREvent(int event);
	void setArg(void* arg);
	void setReadCallback(EventCallback callback);
	void setWriteCallback(EventCallback callback);
	void setErrorCallback(EventCallback callback);

	void enableReadHandling();
	void enableWriteHandling();
	void enableErrorHandling();
	void disableReadeHandling();
	void disableWriteHandling();
	void disableErrorHandling();

	bool isNoneHandling() const;
	bool isReadHandling() const;
	bool isWriteHandling() const;
	bool isErrorHandling() const;

	void handleEvent();

private:
	int _fd;
	void* _arg;
	int _event;
	int _revent;
	EventCallback _readCallback;
	EventCallback _writeCallback;
	EventCallback _errorCallback;
};

#endif