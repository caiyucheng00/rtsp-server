#ifndef __TCPCONNECTION__H_
#define __TCPCONNECTION__H_

#include "../Scheduler/UsageEnvironment.h"
#include "../Scheduler/Event.h"
#include "../Utils/Buffer.h"

class TCPConnection
{
public:
	typedef void (*DisConnectCallback)(void*, int);
	TCPConnection(UsageEnvironment* env, int clientFd);
	virtual ~TCPConnection();

	void setDisConnectCallback(DisConnectCallback callback, void* arg);

protected:
	void enableReadHandling();
	void enableWriteHandling();
	void enableErrorHandling();
	void disableReadeHandling();
	void disableWriteHandling();
	void disableErrorHandling();

	void handleRead();
	virtual void handleReadBytes();
	virtual void handleWrite();
	virtual void handleError();

	void handleDisConnect();

	UsageEnvironment* _env;
	int _clientFd;
	IOEvent* _clientIOEvent;
	DisConnectCallback _disConnectCallback;//在RtspServer实例化该类子类的实例时，设置的回调函数
	void* _arg;
	Buffer _inputBuffer;
	Buffer _outBuffer;
	char _buffer[2048];

private:
	static void readCallback(void* arg);
	static void writeCallback(void* arg);
	static void errorCallback(void* arg);
};

#endif