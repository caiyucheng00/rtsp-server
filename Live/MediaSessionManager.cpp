#include "MediaSessionManager.h"
#include "MediaSession.h"

MediaSessionManager::MediaSessionManager()
{

}

MediaSessionManager::~MediaSessionManager()
{

}

MediaSessionManager* MediaSessionManager::createNew()
{
	return new MediaSessionManager();
}

bool MediaSessionManager::addSession(MediaSession* session)
{
	if (_sessionMap.find(session->getName()) != _sessionMap.end()) { // 已存在
		return false;  
	}
	else
	{
		_sessionMap.insert(std::make_pair(session->getName(), session));
		return true;
	}
}

bool MediaSessionManager::removeSession(MediaSession* session)
{
	auto it = _sessionMap.find(session->getName());
	if (it == _sessionMap.end()) {
		return false;
	}
	else
	{
		_sessionMap.erase(it);
		return true;
	}
}

MediaSession* MediaSessionManager::getSession(const std::string& name)
{
	auto it = _sessionMap.find(name);
	if (it == _sessionMap.end()) {
		return NULL;
	}
	else
	{
		return it->second;
	}
}
