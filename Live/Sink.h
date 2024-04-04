#ifndef __SINK__H_
#define __SINK__H_

#include <string>
#include <stdint.h>
#include "RTPPacket.h"
#include "Source.h"
#include "../Scheduler/Event.h"
#include "../Scheduler/UsageEnvironment.h"

enum PacketType
{
	UNKNOWN = -1,
	RTPPACKET = 0,
};

class Sink
{
public:
	typedef void (*SessionSendPacketCallback)(void* arg1, void* arg2, void* packet, PacketType packetType);
	
	Sink(UsageEnvironment* env, Source* source, int payloadType);
	virtual ~Sink();

	virtual std::string getMediaDescription(uint16_t port) = 0;
	virtual std::string getAttribute() = 0;

	void stopTimerEvent();
	void setSessionCallback(SessionSendPacketCallback callback, void* arg1, void* arg2);

protected:
	virtual void sendFrame(MediaFrame* frame) = 0;
	void sendRTPPacket(RTPPacket* packet);
	void runEvery(int interval);

	UsageEnvironment* _env;
	Source* _source;
	SessionSendPacketCallback _sessionSendPacketCallback;
	void* _arg1;
	void* _arg2;

	uint8_t _csrcLen;
	uint8_t _extension;
	uint8_t _padding;
	uint8_t _version;
	uint8_t _payloadType;
	uint8_t _marker;
	uint16_t _seq;
	uint32_t _timestamp;
	uint32_t _ssrc;

private:
	static void callbackTimeout(void* arg);
	void handleTimeout();

	TimerEvent* _timerEvent;
	Timer::TimerId _timerId;// runEvery()之后获取
};

#endif