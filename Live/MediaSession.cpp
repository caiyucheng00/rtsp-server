#include "MediaSession.h"
#include <algorithm>
#include <assert.h>
#include "../Utils/Log.h"

MediaSession::MediaSession(const std::string& sessionName) :
	_sessionName(sessionName),
	_isStartMulticast(false)
{
	LOGI("MediaSession() name=%s", sessionName.data());

	_tracks[0]._trackId = TrackId0;
	_tracks[0]._isAlive = false;

	_tracks[1]._trackId = TrackId1;
	_tracks[1]._isAlive = false;

	for (int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i) {
		_multicastRtpInstances[i] = NULL;
		_multicastRtcpInstances[i] = NULL;
	}
}

MediaSession::~MediaSession()
{
	LOGI("~MediaSession()");

	for (int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i) {
		if (_multicastRtpInstances[i]) {
			this->removeRtpInstance(_multicastRtpInstances[i]);
			delete _multicastRtpInstances[i];
		}

		if (_multicastRtcpInstances[i]) {
			delete _multicastRtcpInstances[i];
		}
	}

	for (int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i) {
		if (_tracks[i]._isAlive) {
			Sink* sink = _tracks[i]._sink;
			delete sink;
		}
	}
}

MediaSession* MediaSession::createNew(std::string sessionName)
{
	return new MediaSession(sessionName);
}

bool MediaSession::addSink(TrackId trackId, Sink* sink)
{
	Track* track = getTrack(trackId);
	if (!track) return false;

	track->_sink = sink;
	track->_isAlive = true;

	sink->setSessionCallback(MediaSession::sendPacketCallback, this, track);

	return true;
}

bool MediaSession::addRtpInstance(TrackId trackId, RTPInstance* rtpInstance)
{
	Track* track = getTrack(trackId);
	if (!track || track->_isAlive != true) return false;

	track->_rtpInstance_list.push_back(rtpInstance);

	return true;
}

bool MediaSession::removeRtpInstance(RTPInstance* rtpInstance)
{
	for (int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i) {
		if (_tracks[i]._isAlive == false) continue;

		auto it = std::find(_tracks[i]._rtpInstance_list.begin(), _tracks[i]._rtpInstance_list.end(), rtpInstance);
		if (it == _tracks[i]._rtpInstance_list.end()) continue;

		_tracks[i]._rtpInstance_list.erase(it);
		return true;
	}

	return false;
}

std::string MediaSession::getName() const
{
	return _sessionName;
}

std::string MediaSession::generateSDPDescription()
{
	if (!_sdp.empty())
		return _sdp;

	std::string ip = "0.0.0.0";
	char buf[2048] = { 0 };

	snprintf(buf, sizeof(buf),
		"v=0\r\n"
		"o=- 9%ld 1 IN IP4 %s\r\n"
		"t=0 0\r\n"
		"a=control:*\r\n"
		"a=type:broadcast\r\n",
		(long)time(NULL), ip.c_str());

	if (isStartMulticast())
	{
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
			"a=rtcp-unicast: reflection\r\n");
	}

	for (int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
	{
		uint16_t port = 0;

		if (_tracks[i]._isAlive != true)
			continue;

		if (isStartMulticast())
			port = getMulticastDestRtpPort((TrackId)i);

		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
			"%s\r\n", _tracks[i]._sink->getMediaDescription(port).c_str());

		if (isStartMulticast())
			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
				"c=IN IP4 %s/255\r\n", getMulticastDestAddr().c_str());
		else
			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
				"c=IN IP4 0.0.0.0\r\n");

		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
			"%s\r\n", _tracks[i]._sink->getAttribute().c_str());

		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
			"a=control:track%d\r\n", _tracks[i]._trackId);
	}

	_sdp = buf;
	return _sdp;
}

bool MediaSession::startMulticast()
{
	// 随机生成多播地址
	struct sockaddr_in addr = { 0 };
	uint32_t range = 0xE8FFFFFF - 0xE8000100;
	addr.sin_addr.s_addr = htonl(0xE8000100 + (rand()) % range);
	_multicastAddr = inet_ntoa(addr.sin_addr);

	int rtpSockfd1, rtcpSockfd1;
	int rtpSockfd2, rtcpSockfd2;
	uint16_t rtpPort1, rtcpPort1;
	uint16_t rtpPort2, rtcpPort2;
	bool ret;

	rtpSockfd1 = sockets::createUdpSock();
	assert(rtpSockfd1 > 0);

	rtpSockfd2 = sockets::createUdpSock();
	assert(rtpSockfd2 > 0);

	rtcpSockfd1 = sockets::createUdpSock();
	assert(rtcpSockfd1 > 0);

	rtcpSockfd2 = sockets::createUdpSock();
	assert(rtcpSockfd2 > 0);

	uint16_t port = rand() & 0xfffe;
	if (port < 10000)
		port += 10000;

	rtpPort1 = port;
	rtcpPort1 = port + 1;
	rtpPort2 = rtcpPort1 + 1;
	rtcpPort2 = rtpPort2 + 1;

	_multicastRtpInstances[TrackId0] = RTPInstance::createNewOverUdp(rtpSockfd1, 0, _multicastAddr, rtpPort1);
	_multicastRtpInstances[TrackId1] = RTPInstance::createNewOverUdp(rtpSockfd2, 0, _multicastAddr, rtpPort2);
	_multicastRtcpInstances[TrackId0] = RtcpInstance::createNew(rtcpSockfd1, 0, _multicastAddr, rtcpPort1);
	_multicastRtcpInstances[TrackId1] = RtcpInstance::createNew(rtcpSockfd2, 0, _multicastAddr, rtcpPort2);

	this->addRtpInstance(TrackId0, _multicastRtpInstances[TrackId0]);
	this->addRtpInstance(TrackId1, _multicastRtpInstances[TrackId1]);
	_multicastRtpInstances[TrackId0]->setAlive(true);
	_multicastRtpInstances[TrackId1]->setAlive(true);

	_isStartMulticast = true;

	return true;
}

bool MediaSession::isStartMulticast()
{
	return _isStartMulticast;
}

std::string MediaSession::getMulticastDestAddr() const
{
	return _multicastAddr;
}

uint16_t MediaSession::getMulticastDestRtpPort(TrackId trackId)
{
	if (trackId > TrackId1 || !_multicastRtpInstances[trackId]) return -1;

	return _multicastRtpInstances[trackId]->getPeerPort();
}

Track* MediaSession::getTrack(TrackId trackId)
{
	for (int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i) {
		if (_tracks[i]._trackId == trackId) {
			return &_tracks[i];
		}
	}

	return NULL;
}

void MediaSession::sendPacketCallback(void* arg1, void* arg2, void* packet, PacketType packetType)
{
	RTPPacket* rtpPacket = (RTPPacket*)packet;
	MediaSession* session = (MediaSession*)arg1;
	Track* track = (Track*)arg2;

	session->handleSendRtpPacket(track, rtpPacket);
}

void MediaSession::handleSendRtpPacket(Track* tarck, RTPPacket* rtpPacket)
{
	for (auto it = tarck->_rtpInstance_list.begin(); it != tarck->_rtpInstance_list.end(); ++it) {
		if ((*it)->getAlive()) {
			(*it)->send(rtpPacket);
		}
	}
}
