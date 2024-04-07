#include "RTSPServer.h"
#include "RTSPConnection.h"
#include "../Utils/Log.h"

RTSPServer::RTSPServer(UsageEnvironment* env, MediaSessionManager* sessMgr, Ipv4Address& addr) :
	_env(env),
	_sessMgr(sessMgr),
	_addr(addr),
	_listen(false)
{
	_fd = sockets::createTcpSock(); // 非阻塞
	sockets::setReuseAddr(_fd, 1); //setsockopt 快速重启端口不占用
	if (!sockets::bind(_fd, addr.getIp(), _addr.getPort())) {   // 绑定
		return;
	}
	LOGI("rtsp://%s:%d fd=%d", addr.getIp().data(), addr.getPort(), _fd);

	_acceptIOEvent = IOEvent::createNew(_fd, this);           //客户端IO在rtsp/tcpconnection类
	_acceptIOEvent->setReadCallback(readCallback);         // 设置回调
	_acceptIOEvent->enableReadHandling();

	_closeTriggerEvent = TriggerEvent::createNew(this);   // 只此一次
	_closeTriggerEvent->setTriggerCallback(closeConnectCallback); // 设置回调

}

RTSPServer::~RTSPServer()
{
	if (_listen) {
		_env->getScheduler()->removeIOEvent(_acceptIOEvent);
	}

	delete _acceptIOEvent;
	delete _closeTriggerEvent;

	sockets::close(_fd);                     // 关闭
}

RTSPServer* RTSPServer::createNew(UsageEnvironment* env, MediaSessionManager* sessMgr, Ipv4Address& addr)
{
	return new RTSPServer(env, sessMgr, addr);
}

void RTSPServer::start()
{
	LOGI("RTSPServer::start()");
	_listen = true;      // 初始化为false
	sockets::listen(_fd, 60);   // 监听
	_env->getScheduler()->addIOEvent(_acceptIOEvent);   // select加入
}

UsageEnvironment* RTSPServer::getEnv() const
{
	return _env;
}

MediaSessionManager* RTSPServer::getSessMgr() const
{
	return _sessMgr;
}

void RTSPServer::readCallback(void* arg)
{
	RTSPServer* rtspServer = (RTSPServer*)arg;
	rtspServer->handleRead();
}

void RTSPServer::handleRead()
{
	int clientFd = sockets::accept(_fd);                         // 来链接-》创建链接类，设置关闭链接事件
	if (clientFd < 0) {
		LOGE("handleRead error,clientFd=%d", clientFd);
		return;
	}

	RTSPConnection* conn = RTSPConnection::createNew(this, clientFd);
	conn->setDisConnectCallback(RTSPServer::disconnectCallback, this);
	_connMap.insert(std::make_pair(clientFd, conn));
}

void RTSPServer::disconnectCallback(void* arg, int clientFd)
{
	RTSPServer* rtspServer = (RTSPServer*)arg;
	rtspServer->handleDisconnect(clientFd);
}

void RTSPServer::handleDisconnect(int clientFd)
{
	std::lock_guard<std::mutex> lck(_mutex);
	_disconn_list.push_back(clientFd);          // 加入取消队列

	_env->getScheduler()->addTriggerEvent(_closeTriggerEvent);
}

void RTSPServer::closeConnectCallback(void* arg)
{
	RTSPServer* rtspServer = (RTSPServer*)arg;
	rtspServer->handleCloseConnect();
}

void RTSPServer::handleCloseConnect()
{
	std::lock_guard<std::mutex> lck(_mutex);
	for (auto it = _disconn_list.begin(); it != _disconn_list.end(); ++it) {
		int clientFd = *it;
		auto it_map = _connMap.find(clientFd);
		assert(it_map != _connMap.end());
		delete it_map->second;
		_connMap.erase(clientFd);
	}

	_disconn_list.clear();
}
