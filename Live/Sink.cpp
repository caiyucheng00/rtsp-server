#include "Sink.h"
#include <WinSock2.h>
#include <WS2tcpip.h>

Sink::Sink(UsageEnvironment* env, Source* source, int payloadType) :
	_source(source),
	_env(env),
	_csrcLen(0),
	_extension(0),
	_padding(0),
	_version(RTP_VESION),
	_payloadType(payloadType),
	_marker(0),
	_seq(0),
	_ssrc(rand()),
	_timestamp(0),
	_timerId(0),
	_sessionSendPacketCallback(NULL),
	_arg1(NULL),
	_arg2(NULL)
{
	_timerEvent = TimerEvent::createNew(this);
	_timerEvent->setTimeoutCallback(callbackTimeout);
}

Sink::~Sink() 
{
	delete _timerEvent;
	delete _source;
}

void Sink::stopTimerEvent()
{
	_timerEvent->stop();
}

void Sink::setSessionCallback(SessionSendPacketCallback callback, void* arg1, void* arg2)
{
	_sessionSendPacketCallback = callback;
	_arg1 = arg1;
	_arg2 = arg2;
}

void Sink::sendRTPPacket(RTPPacket* packet)
{
	RtpHeader* rtpHeader = packet->_rtpHeader;
	rtpHeader->csrcLen = _csrcLen;
	rtpHeader->extension = _extension;
	rtpHeader->padding = _padding;
	rtpHeader->version = _version;
	rtpHeader->payloadType = _payloadType;
	rtpHeader->marker = _marker;
	rtpHeader->seq = htons(_seq);
	rtpHeader->timestamp = htonl(_timestamp);
	rtpHeader->ssrc = htonl(_ssrc);

	if (_sessionSendPacketCallback) {
		_sessionSendPacketCallback(_arg1, _arg2, packet, PacketType::RTPPACKET);
	}
}

void Sink::runEvery(int interval)
{
	_env->getScheduler()->addTimerEventRunEvery(_timerEvent, interval);
}

void Sink::callbackTimeout(void* arg)
{
	Sink* sink = (Sink*)arg;
	sink->handleTimeout();
}

void Sink::handleTimeout()
{
	MediaFrame* frame = _source->getFrameFromOutputQueue();
	if (!frame) return;

	this->sendFrame(frame);  // 具体子类
	//将使用过的frame插入输入队列，插入输入队列以后，加入一个子线程task，从文件中读取数据再次将输入写入到frame
	_source->putFrameToInputQueue(frame);
}
