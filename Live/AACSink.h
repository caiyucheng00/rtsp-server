#ifndef __AACSINK__H_
#define __AACSINK__H_

#include "Sink.h"

static uint32_t AACSampleRate[16] =
{
	97000, 88200, 64000, 48000,
	44100, 32000, 24000, 22050,
	16000, 12000, 11025, 8000,
	7350, 0, 0, 0 /*reserved */
};

class AACSink : public Sink
{
public:
public:
	AACSink(UsageEnvironment* env, Source* source);
	virtual ~AACSink();
	static AACSink* createNew(UsageEnvironment* env, Source* source);

	virtual std::string getMediaDescription(uint16_t port) override;
	virtual std::string getAttribute() override;
	virtual void sendFrame(MediaFrame* frame) override;

private:
	RTPPacket _packet;
	uint32_t _sampleRate;   // 采样频率
	uint32_t _channels;         // 通道数
	int _fps;
};

#endif