#include "Sink.h"

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
}

Sink::~Sink() 
{

}

void Sink::stopTimerEvent()
{

}

void Sink::setSessionCallback(SessionSendPacketCallback callback, void* arg1, void* arg2)
{

}

void Sink::sendRTPPacket(RTPPacket* packet)
{

}

void Sink::runEvery(int interval)
{

}

void Sink::callbackTimeout(void* arg)
{

}

void Sink::handleTimeout()
{

}
