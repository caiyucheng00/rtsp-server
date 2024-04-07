#include "RTSPConnection.h"
#include "RTSPServer.h"
#include "MediaSessionManager.h"
#include "../Utils/Log.h"
#include "../Utils/Version.h"

static void getPeerIp(int fd, std::string& ip)
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	getpeername(fd, (struct sockaddr*)&addr, &addrlen);
	ip = inet_ntoa(addr.sin_addr);
}

RTSPConnection::RTSPConnection(RTSPServer* rtspServer, int clientFd) :
	TCPConnection(rtspServer->getEnv(), clientFd),
	_rtspServer(rtspServer),
	_method(RTSPConnection::Method::NONE),
	_trackId(TrackId::TrackIdNone),
	_sessionId(rand()),
	_isRtpOverTcp(false),
	_streamPrefix("track")
{
	LOGI("RtspConnection() ClientFd=%d", _clientFd);

	for (int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
	{
		_rtpInstances[i] = NULL;
		_rtcpInstances[i] = NULL;
	}

	getPeerIp(clientFd, _peerIp);
}

RTSPConnection::~RTSPConnection()
{
	LOGI("~RtspConnection() ClientFd=%d", _clientFd);

	for (int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
	{
		if (_rtpInstances[i])
		{
			MediaSession* session = _rtspServer->getSessMgr()->getSession(_suffix);

			if (!session) {
				session->removeRtpInstance(_rtpInstances[i]);
			}
			delete _rtpInstances[i];
		}

		if (_rtcpInstances[i])
		{
			delete _rtcpInstances[i];
		}
	}
}

RTSPConnection* RTSPConnection::createNew(RTSPServer* rtspServer, int clientFd)
{
	return new RTSPConnection(rtspServer, clientFd);
}

void RTSPConnection::handleReadBytes()
{
	if (_isRtpOverTcp) {
		if (_inputBuffer.peek()[0] == '$') {
			handleRtpOverTcp();   //TCP
			return;
		}
	}

	if (!parseRequest()) {
		LOGE("parseRequest err");
		goto disConnect;
	}

	switch (_method)
	{
	case OPTIONS:
		if (!handleCmdOption())
			goto disConnect;
		break;
	case DESCRIBE:
		if (!handleCmdDescribe())
			goto disConnect;
		break;
	case SETUP:
		if (!handleCmdSetup())
			goto disConnect;
		break;
	case PLAY:
		if (!handleCmdPlay())
			goto disConnect;
		break;
	case TEARDOWN:
		if (!handleCmdTeardown())
			goto disConnect;
		break;

	default:
		goto disConnect;
	}

	return;

disConnect:
	handleDisConnect();
}

bool RTSPConnection::parseRequest()
{
	//解析第一行
	const char* crlf = _inputBuffer.findCRLF();
	if (crlf == NULL) {
		_inputBuffer.retrieveAll();
		return false;
	}
	bool ret = parseRequest1(_inputBuffer.peek(), crlf);
	if (ret == false) {
		_inputBuffer.retrieveAll();
		return false;
	}
	else {
		_inputBuffer.retrieveUntil(crlf + 2);
	}

	//解析第一行之后的所有行
	crlf = _inputBuffer.findLastCrlf();
	if (crlf == NULL)
	{
		_inputBuffer.retrieveAll();
		return false;
	}
	ret = parseRequest2(_inputBuffer.peek(), crlf);

	if (ret == false)
	{
		_inputBuffer.retrieveAll();
		return false;
	}
	else {
		_inputBuffer.retrieveUntil(crlf + 2);
		return true;
	}
}

bool RTSPConnection::parseRequest1(const char* begin, const char* end)
{
	std::string message(begin, end);
	char method[64] = { 0 };
	char url[512] = { 0 };
	char version[64] = { 0 };

	if (sscanf(message.c_str(), "%s %s %s", method, url, version) != 3)
	{
		return false;
	}

	if (!strcmp(method, "OPTIONS")) {
		_method = OPTIONS;
	}
	else if (!strcmp(method, "DESCRIBE")) {
		_method = DESCRIBE;
	}
	else if (!strcmp(method, "SETUP")) {
		_method = SETUP;
	}
	else if (!strcmp(method, "PLAY")) {
		_method = PLAY;
	}
	else if (!strcmp(method, "TEARDOWN")) {
		_method = TEARDOWN;
	}
	else {
		_method = NONE;
		return false;
	}
	if (strncmp(url, "rtsp://", 7) != 0)
	{
		return false;
	}

	uint16_t port = 0;
	char ip[64] = { 0 };
	char suffix[64] = { 0 };

	if (sscanf(url + 7, "%[^:]:%hu/%s", ip, &port, suffix) == 3)
	{

	}
	else if (sscanf(url + 7, "%[^/]/%s", ip, suffix) == 2)
	{
		port = 554;// 如果rtsp请求地址中无端口，默认获取的端口为：554
	}
	else
	{
		return false;
	}

	_url = url;
	_suffix = suffix;

	return true;
}

bool RTSPConnection::parseRequest2(const char* begin, const char* end)
{
	std::string message(begin, end);

	if (!parseCSeq(message)) {
		return false;
	}
	if (_method == OPTIONS) {
		return true;
	}
	else if (_method == DESCRIBE) {
		return parseDescribe(message);
	}
	else if (_method == SETUP)
	{
		return parseSetup(message);
	}
	else if (_method == PLAY) {
		return parsePlay(message);
	}
	else if (_method == TEARDOWN) {
		return true;
	}
	else {
		return false;
	}
}

bool RTSPConnection::parseCSeq(std::string& message)
{
	std::size_t pos = message.find("CSeq");
	if (pos != std::string::npos)
	{
		uint32_t cseq = 0;
		sscanf(message.c_str() + pos, "%*[^:]: %u", &cseq);
		_cseq = cseq;
		return true;
	}

	return false;
}

bool RTSPConnection::parseDescribe(std::string& message)
{
	if ((message.rfind("Accept") == std::string::npos)
		|| (message.rfind("sdp") == std::string::npos))
	{
		return false;
	}

	return true;
}

bool RTSPConnection::parseSetup(std::string& message)
{
	_trackId = TrackIdNone;
	std::size_t pos;

	for (int i = 0; i < MEDIA_MAX_TRACK_NUM; i++) {                      // 填充——trackID
		pos = _url.find(_streamPrefix + std::to_string(i));
		if (pos != std::string::npos)
		{
			if (i == 0) {
				_trackId = TrackId0;
			}
			else if (i == 1)
			{
				_trackId = TrackId1;
			}
		}
	}

	if (_trackId == TrackIdNone) {
		return false;
	}

	pos = message.find("Transport");                                     //判断TCP/UDP
	if (pos != std::string::npos)
	{
		if ((pos = message.find("RTP/AVP/TCP")) != std::string::npos)
		{
			uint8_t rtpChannel, rtcpChannel;
			_isRtpOverTcp = true;

			if (sscanf(message.c_str() + pos, "%*[^;];%*[^;];%*[^=]=%hhu-%hhu",
				&rtpChannel, &rtcpChannel) != 2)
			{
				return false;
			}

			_rtpChannel = rtpChannel;

			return true;
		}
		else if ((pos = message.find("RTP/AVP")) != std::string::npos)
		{
			uint16_t rtpPort = 0, rtcpPort = 0;
			if (((message.find("unicast", pos)) != std::string::npos))
			{
				if (sscanf(message.c_str() + pos, "%*[^;];%*[^;];%*[^=]=%hu-%hu",
					&rtpPort, &rtcpPort) != 2)
				{
					return false;
				}
			}
			else if ((message.find("multicast", pos)) != std::string::npos)
			{
				return true;
			}
			else
				return false;

			_peerRtpPort = rtpPort;
			_peerRtcpPort = rtcpPort;
		}
		else
		{
			return false;
		}

		return true;
	}

	return false;
}

bool RTSPConnection::parsePlay(std::string& message)
{
	std::size_t pos = message.find("Session");
	if (pos != std::string::npos)
	{
		uint32_t sessionId = 0;
		if (sscanf(message.c_str() + pos, "%*[^:]: %u", &sessionId) != 1)
			return false;
		return true;
	}

	return false;
}

bool RTSPConnection::handleCmdOption()
{
	snprintf(_buffer, sizeof(_buffer),
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %u\r\n"
		"Public: DESCRIBE, ANNOUNCE, SETUP, PLAY, RECORD, PAUSE, GET_PARAMETER, TEARDOWN\r\n"
		"Server: %s\r\n"
		"\r\n", _cseq, PROJECT_VERSION);

	if (sendMessage(_buffer, strlen(_buffer)) < 0)
		return false;

	return true;

}

bool RTSPConnection::handleCmdDescribe()
{
	MediaSession* session = _rtspServer->getSessMgr()->getSession(_suffix);

	if (!session) {
		LOGE("can't find session:%s", _suffix.c_str());
		return false;
	}
	std::string sdp = session->generateSDPDescription();

	memset((void*)_buffer, 0, sizeof(_buffer));
	snprintf((char*)_buffer, sizeof(_buffer),
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %u\r\n"
		"Content-Length: %u\r\n"
		"Content-Type: application/sdp\r\n"
		"\r\n"
		"%s",
		_cseq,
		(unsigned int)sdp.size(),
		sdp.c_str());

	if (sendMessage(_buffer, strlen(_buffer)) < 0)
		return false;

	return true;
}

bool RTSPConnection::handleCmdSetup()
{
	char sessionName[100];
	if (sscanf(_suffix.c_str(), "%[^/]/", sessionName) != 1)
	{
		return false;
	}
	MediaSession* session = _rtspServer->getSessMgr()->getSession(sessionName);
	if (!session) {
		LOGE("can't find session:%s", sessionName);
		return false;
	}

	if (_trackId >= MEDIA_MAX_TRACK_NUM || _rtpInstances[_trackId] || _rtcpInstances[_trackId]) {
		return false;
	}

	if (session->isStartMulticast()) {
		snprintf((char*)_buffer, sizeof(_buffer),
			"RTSP/1.0 200 OK\r\n"
			"CSeq: %d\r\n"
			"Transport: RTP/AVP;multicast;"
			"destination=%s;source=%s;port=%d-%d;ttl=255\r\n"
			"Session: %08x\r\n"
			"\r\n",
			_cseq,
			session->getMulticastDestAddr().c_str(),
			sockets::getLocalIp().c_str(),
			session->getMulticastDestRtpPort(_trackId),
			session->getMulticastDestRtpPort(_trackId) + 1,
			_sessionId);
	}
	else {
		if (_isRtpOverTcp)
		{
			//创建rtp over tcp
			createRtpOverTcp(_trackId, _clientFd, _rtpChannel);
			_rtpInstances[_trackId]->setSessionId(_sessionId);

			session->addRtpInstance(_trackId, _rtpInstances[_trackId]);

			snprintf((char*)_buffer, sizeof(_buffer),
				"RTSP/1.0 200 OK\r\n"
				"CSeq: %d\r\n"
				"Server: %s\r\n"
				"Transport: RTP/AVP/TCP;unicast;interleaved=%hhu-%hhu\r\n"
				"Session: %08x\r\n"
				"\r\n",
				_cseq, PROJECT_VERSION,
				_rtpChannel,
				_rtpChannel + 1,
				_sessionId);
		}
		else
		{
			//创建 rtp over udp
			if (createRtpRtcpOverUdp(_trackId, _peerIp, _peerRtpPort, _peerRtcpPort) != true)
			{
				LOGE("failed to createRtpRtcpOverUdp");
				return false;
			}

			_rtpInstances[_trackId]->setSessionId(_sessionId);
			_rtcpInstances[_trackId]->setSessionId(_sessionId);

			session->addRtpInstance(_trackId, _rtpInstances[_trackId]);

			snprintf((char*)_buffer, sizeof(_buffer),
				"RTSP/1.0 200 OK\r\n"
				"CSeq: %u\r\n"
				"Server: %s\r\n"
				"Transport: RTP/AVP;unicast;client_port=%hu-%hu;server_port=%hu-%hu\r\n"
				"Session: %08x\r\n"
				"\r\n",
				_cseq, PROJECT_VERSION,
				_peerRtpPort,
				_peerRtcpPort,
				_rtpInstances[_trackId]->getLocalPort(),
				_rtcpInstances[_trackId]->getLocalPort(),
				_sessionId);
		}

	}

	if (sendMessage(_buffer, strlen(_buffer)) < 0)
		return false;

	return true;
}

bool RTSPConnection::handleCmdPlay()
{
	snprintf((char*)_buffer, sizeof(_buffer),
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Server: %s\r\n"
		"Range: npt=0.000-\r\n"
		"Session: %08x; timeout=60\r\n"
		"\r\n",
		_cseq, PROJECT_VERSION,
		_sessionId);

	if (sendMessage(_buffer, strlen(_buffer)) < 0)
		return false;

	for (int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
	{
		if (_rtpInstances[i]) {
			_rtpInstances[i]->setAlive(true);
		}

		if (_rtcpInstances[i]) {
			_rtcpInstances[i]->setAlive(true);
		}

	}

	return true;
}

bool RTSPConnection::handleCmdTeardown()
{
	snprintf((char*)_buffer, sizeof(_buffer),
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Server: %s\r\n"
		"\r\n",
		_cseq, PROJECT_VERSION);

	if (sendMessage(_buffer, strlen(_buffer)) < 0)
	{
		return false;
	}

	return true;
}

int RTSPConnection::sendMessage(void* buf, int size)
{
	LOGI("%s", buf);
	int ret;

	_outBuffer.append(buf, size);
	ret = _outBuffer.write(_clientFd);
	_outBuffer.retrieveAll();

	return ret;
}

int RTSPConnection::sendMessage()
{
	int ret = _outBuffer.write(_clientFd);
	_outBuffer.retrieveAll();
	return ret;
}

bool RTSPConnection::createRtpRtcpOverUdp(TrackId trackId, std::string peerIp, uint16_t peerRtpPort, uint16_t peerRtcpPort)
{
	int rtpSockfd, rtcpSockfd;
	int16_t rtpPort, rtcpPort;
	bool ret;

	if (_rtpInstances[trackId] || _rtcpInstances[trackId])
		return false;

	int i;
	for (i = 0; i < 10; ++i) {// 重试10次
		rtpSockfd = sockets::createUdpSock();
		if (rtpSockfd < 0)
		{
			return false;
		}

		rtcpSockfd = sockets::createUdpSock();
		if (rtcpSockfd < 0)
		{
			sockets::close(rtpSockfd);
			return false;
		}

		uint16_t port = rand() & 0xfffe;
		if (port < 10000)
			port += 10000;

		rtpPort = port;
		rtcpPort = port + 1;

		ret = sockets::bind(rtpSockfd, "0.0.0.0", rtpPort);
		if (ret != true)
		{
			sockets::close(rtpSockfd);
			sockets::close(rtcpSockfd);
			continue;
		}

		ret = sockets::bind(rtcpSockfd, "0.0.0.0", rtcpPort);
		if (ret != true)
		{
			sockets::close(rtpSockfd);
			sockets::close(rtcpSockfd);
			continue;
		}

		break;
	}

	if (i == 10)
		return false;

	_rtpInstances[trackId] = RTPInstance::createNewOverUdp(rtpSockfd, rtpPort, peerIp, peerRtpPort);
	_rtcpInstances[trackId] = RtcpInstance::createNew(rtcpSockfd, rtcpPort, peerIp, peerRtcpPort);

	return true;
}

bool RTSPConnection::createRtpOverTcp(TrackId trackId, int sockfd, uint8_t rtpChannel)
{
	_rtpInstances[trackId] = RTPInstance::createNewOverTcp(sockfd, rtpChannel);

	return true;
}

void RTSPConnection::handleRtpOverTcp()
{
	int num = 0;
	while (true)
	{
		num += 1;
		uint8_t* buf = (uint8_t*)_inputBuffer.peek();
		uint8_t rtpChannel = buf[1];
		int16_t rtpSize = (buf[2] << 8) | buf[3];

		int16_t bufSize = 4 + rtpSize;

		if (_inputBuffer.readableBytes() < bufSize) {
			// 缓存数据小于一个RTP数据包的长度
			return;
		}
		else {
			if (0x00 == rtpChannel) {
				RtpHeader rtpHeader;
				parseRtpHeader(buf + 4, &rtpHeader);
				LOGI("num=%d,rtpSize=%d", num, rtpSize);
			}
			else if (0x01 == rtpChannel)
			{
				RtcpHeader rtcpHeader;
				parseRtcpHeader(buf + 4, &rtcpHeader);

				LOGI("num=%d,rtcpHeader.packetType=%d,rtpSize=%d", num, rtcpHeader.packetType, rtpSize);
			}
			else if (0x02 == rtpChannel) {
				RtpHeader rtpHeader;
				parseRtpHeader(buf + 4, &rtpHeader);
				LOGI("num=%d,rtpSize=%d", num, rtpSize);
			}
			else if (0x03 == rtpChannel)
			{
				RtcpHeader rtcpHeader;
				parseRtcpHeader(buf + 4, &rtcpHeader);

				LOGI("num=%d,rtcpHeader.packetType=%d,rtpSize=%d", num, rtcpHeader.packetType, rtpSize);
			}

			_inputBuffer.retrieve(bufSize);
		}
	}
}
