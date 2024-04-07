#include "TCPConnection.h"
#include "../Utils/SocketsOps.h"
#include "../Utils/Log.h"

TCPConnection::TCPConnection(UsageEnvironment* env, int clientFd) :
	_env(env),
	_clientFd(clientFd)
{
	_clientIOEvent = IOEvent::createNew(clientFd, this);
	_clientIOEvent->setReadCallback(readCallback);       // rtsp->handleread
	_clientIOEvent->setWriteCallback(writeCallback);
	_clientIOEvent->setErrorCallback(errorCallback);

	_clientIOEvent->enableReadHandling();  //默认只开启读

	_env->getScheduler()->addIOEvent(_clientIOEvent);   // select加入
}

TCPConnection::~TCPConnection()
{
	_env->getScheduler()->removeIOEvent(_clientIOEvent);
	delete _clientIOEvent;

	sockets::close(_clientFd);
}

void TCPConnection::setDisConnectCallback(DisConnectCallback callback, void* arg)
{
	_disConnectCallback = callback;
	_arg = arg;
}

void TCPConnection::enableReadHandling()
{
	if (_clientIOEvent->isReadHandling()) return;

	_clientIOEvent->enableReadHandling();
	_env->getScheduler()->updateIOEvent(_clientIOEvent);
}

void TCPConnection::enableWriteHandling()
{
	if (_clientIOEvent->isWriteHandling()) return;

	_clientIOEvent->enableWriteHandling();
	_env->getScheduler()->updateIOEvent(_clientIOEvent);
}

void TCPConnection::enableErrorHandling()
{
	if (_clientIOEvent->isErrorHandling()) return;

	_clientIOEvent->enableErrorHandling();
	_env->getScheduler()->updateIOEvent(_clientIOEvent);
}

void TCPConnection::disableReadeHandling()
{
	if (!_clientIOEvent->isReadHandling()) return;

	_clientIOEvent->disableReadeHandling();
	_env->getScheduler()->updateIOEvent(_clientIOEvent);
}

void TCPConnection::disableWriteHandling()
{
	if (!_clientIOEvent->isWriteHandling()) return;

	_clientIOEvent->disableWriteHandling();
	_env->getScheduler()->updateIOEvent(_clientIOEvent);
}


void TCPConnection::disableErrorHandling()
{
	if (!_clientIOEvent->isErrorHandling()) return;

	_clientIOEvent->disableErrorHandling();
	_env->getScheduler()->updateIOEvent(_clientIOEvent);
}

void TCPConnection::handleRead()
{
	int ret = _inputBuffer.read(_clientFd);
	if (ret <= 0) {
		LOGE("read error,fd=%d,ret=%d", _clientFd, ret);
		handleDisConnect();
		return;
	}

	handleReadBytes();// 调用RtspConnecton对象的实现函数 
}

void TCPConnection::handleReadBytes()
{
	_inputBuffer.retrieveAll();
}

void TCPConnection::handleWrite()
{
	_outBuffer.retrieveAll();
}

void TCPConnection::handleError()
{

}

void TCPConnection::handleDisConnect()
{
	if (_disConnectCallback) {
		_disConnectCallback(_arg, _clientFd);
	}
}

void TCPConnection::readCallback(void* arg)
{
	TCPConnection* conn = (TCPConnection*)arg;
	conn->handleRead();
}

void TCPConnection::writeCallback(void* arg)
{
	TCPConnection* conn = (TCPConnection*)arg;
	conn->handleWrite();
}

void TCPConnection::errorCallback(void* arg)
{
	TCPConnection* conn = (TCPConnection*)arg;
	conn->handleError();
}
