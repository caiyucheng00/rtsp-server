#ifndef __RTPINSTANCE__H_
#define __RTPINSTANCE__H_

#include <string>
#include <stdint.h>
#include "RTPPacket.h"
#include "InetAddress.h"
#include "../Utils/SocketsOps.h"

class RTPInstance
{
public:
	enum RtpType
	{
		RTP_OVER_UDP,
		RTP_OVER_TCP
	};

	RTPInstance(int localSockfd, uint16_t localPort, const std::string& destIp, uint16_t destPort);
	RTPInstance(int sockfd, uint8_t rtpChannel);
	~RTPInstance();
	static RTPInstance* createNewOverUdp(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort);
	static RTPInstance* createNewOverTcp(int sockfd, uint8_t rtpChannel);

	int send(RTPPacket* rtpPacket);

	uint16_t getLocalPort() const;
	uint16_t getPeerPort();
	bool getAlive() const;
	int setAlive(bool alive);
	void setSessionId(uint16_t sessionId);
	uint16_t getSessionId() const;

private:
	int sendOverUdp(void* buf, int size);
	int sendOverTcp(void* buf, int size);

	RtpType _rtpType;
	int _sockFd;
	uint16_t _localPort;  //udp;
	Ipv4Address _destAddr; //udp;
	bool _isAlive;
	uint16_t _sessionId;
	uint8_t _rtpChannel;
};

class RtcpInstance
{
public:
	RtcpInstance(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort);
	~RtcpInstance();
	static RtcpInstance* createNew(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort);

	int send(void* buf, int size);

	uint16_t getLocalPort() const;
	int getAlive() const;
	int setAlive(bool alive);
	void setSessionId(uint16_t sessionId);
	uint16_t getSessionId() const;

private:
	int _localSockfd;
	uint16_t _localPort;
	Ipv4Address _destAddr;
	bool _isAlive;
	uint16_t _sessionId;
};

#endif