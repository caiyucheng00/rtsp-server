#ifndef __RTSPSERVER__H_
#define __RTSPSERVER__H_

#include <mutex>
#include "../Scheduler/UsageEnvironment.h"
#include "../Scheduler/Event.h"
#include "MediaSession.h"
#include "InetAddress.h"

class MediaSessionManager;
class RTSPConnection;

class RTSPServer
{
public:
	RTSPServer(UsageEnvironment* env, MediaSessionManager* sessMgr, Ipv4Address& addr);
	~RTSPServer();
	static RTSPServer* createNew(UsageEnvironment* env, MediaSessionManager* sessMgr, Ipv4Address& addr);

	void start();

	UsageEnvironment* getEnv() const;
	MediaSessionManager* getSessMgr() const;

private:
	static void readCallback(void* arg);
	void handleRead();
	static void disconnectCallback(void* arg, int clientFd);
	void handleDisconnect(int clientFd);
	static void closeConnectCallback(void* arg);
	void handleCloseConnect();

	UsageEnvironment* _env;
	MediaSessionManager* _sessMgr;
	int _fd;
	Ipv4Address _addr;
	bool _listen;
	IOEvent* _acceptIOEvent;
	TriggerEvent* _closeTriggerEvent;

	std::mutex _mutex;
	std::map<int, RTSPConnection*> _connMap;                 //<fd, conn>
	std::vector<int> _disconn_list;                         // fd
};

#endif