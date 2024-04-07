#ifndef __RTSPCONNECTION__H_
#define __RTSPCONNECTION__H_

#include <string>
#include <map>
#include "MediaSession.h"
#include "TCPConnection.h"

class RTSPServer;

class RTSPConnection : public TCPConnection
{
public:
	enum Method
	{
		OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN,
		NONE,
	};

	RTSPConnection(RTSPServer* rtspServer, int clientFd);
	virtual ~RTSPConnection();
	static RTSPConnection* createNew(RTSPServer* rtspServer, int clientFd);

protected:
	virtual void handleReadBytes() override;

private:
	bool parseRequest();
	bool parseRequest1(const char* begin, const char* end);
	bool parseRequest2(const char* begin, const char* end);

	bool parseCSeq(std::string& message);
	bool parseDescribe(std::string& message);
	bool parseSetup(std::string& message);
	bool parsePlay(std::string& message);

	bool handleCmdOption();
	bool handleCmdDescribe();
	bool handleCmdSetup();
	bool handleCmdPlay();
	bool handleCmdTeardown();

	int sendMessage(void* buf, int size);
	int sendMessage();

	bool createRtpRtcpOverUdp(TrackId trackId, std::string peerIp, uint16_t peerRtpPort, uint16_t peerRtcpPort);
	bool createRtpOverTcp(TrackId trackId, int sockfd, uint8_t rtpChannel);

	void handleRtpOverTcp();

	RTSPServer* _rtspServer;
	std::string _peerIp;
	Method _method;
	std::string _url;
	std::string _suffix;
	uint32_t _cseq;
	std::string _streamPrefix;

	uint16_t _peerRtpPort;
	uint16_t _peerRtcpPort;

	TrackId _trackId;
	RTPInstance* _rtpInstances[MEDIA_MAX_TRACK_NUM];
	RtcpInstance* _rtcpInstances[MEDIA_MAX_TRACK_NUM];

	int _sessionId;
	bool _isRtpOverTcp;
	uint8_t _rtpChannel;
};

#endif