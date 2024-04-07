#include "RTPInstance.h"

RTPInstance::RTPInstance(int localSockfd, uint16_t localPort, const std::string& destIp, uint16_t destPort) :
	_rtpType(RTP_OVER_UDP),
	_sockFd(localSockfd),
	_localPort(localPort),
	_destAddr(destIp, destPort),
	_isAlive(false),
	_sessionId(0),
	_rtpChannel(0)
{

}

RTPInstance::RTPInstance(int sockfd, uint8_t rtpChannel) :
	_rtpType(RTP_OVER_TCP),
	_sockFd(sockfd),
	_localPort(0),
	_isAlive(false),
	_sessionId(0),
	_rtpChannel(rtpChannel)
{

}

RTPInstance::~RTPInstance() 
{
	sockets::close(_sockFd);
}

RTPInstance* RTPInstance::createNewOverUdp(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort)
{
	return new RTPInstance(localSockfd, localPort, destIp, destPort);
}

RTPInstance* RTPInstance::createNewOverTcp(int sockfd, uint8_t rtpChannel)
{
	return new RTPInstance(sockfd, rtpChannel);
}

int RTPInstance::send(RTPPacket* rtpPacket)
{
	switch (_rtpType)
	{
	case RTPInstance::RTP_OVER_UDP: {
		return sendOverUdp(rtpPacket->_buf4, rtpPacket->_size);
		break;
	}
	case RTPInstance::RTP_OVER_TCP: {
		rtpPacket->_buf[0] = '$';
		rtpPacket->_buf[1] = (uint8_t)_rtpChannel;
		rtpPacket->_buf[2] = (uint8_t)(((rtpPacket->_size) & 0xFF00) >> 8);
		rtpPacket->_buf[3] = (uint8_t)((rtpPacket->_size) & 0xFF);
		return sendOverTcp(rtpPacket->_buf, 4 + rtpPacket->_size);
		break;
	}

	default: {
		return -1;
		break;
	}
	}
}

uint16_t RTPInstance::getLocalPort() const
{
	return _localPort;
}

uint16_t RTPInstance::getPeerPort()
{
	return _destAddr.getPort();
}

bool RTPInstance::getAlive() const
{
	return _isAlive;
}

int RTPInstance::setAlive(bool alive)
{
	_isAlive = alive;
	return 0;
}

void RTPInstance::setSessionId(uint16_t sessionId)
{
	_sessionId = sessionId;
}

uint16_t RTPInstance::getSessionId() const
{
	return _sessionId;
}

int RTPInstance::sendOverUdp(void* buf, int size)
{
	return sockets::sendto(_sockFd, buf, size, _destAddr.getAddr());
}

int RTPInstance::sendOverTcp(void* buf, int size)
{
	return sockets::write(_sockFd, buf, size);
}

RtcpInstance::RtcpInstance(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort) :
	_localSockfd(localSockfd),
	_localPort(localPort),
	_destAddr(destIp, destPort),
	_isAlive(false),
	_sessionId(0)
{

}

RtcpInstance::~RtcpInstance()
{
	sockets::close(_localSockfd);
}

RtcpInstance* RtcpInstance::createNew(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort)
{
	return new RtcpInstance(localSockfd, localPort, destIp, destPort);
}

int RtcpInstance::send(void* buf, int size)
{
	return sockets::sendto(_localSockfd, buf, size, _destAddr.getAddr());
}

uint16_t RtcpInstance::getLocalPort() const
{
	return _localPort;
}

int RtcpInstance::getAlive() const
{
	return _isAlive;
}

int RtcpInstance::setAlive(bool alive)
{
	_isAlive = alive;
	return 0;
}

void RtcpInstance::setSessionId(uint16_t sessionId)
{
	_sessionId = sessionId;
}

uint16_t RtcpInstance::getSessionId() const
{
	return _sessionId;
}
