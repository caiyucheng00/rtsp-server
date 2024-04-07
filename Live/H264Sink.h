#ifndef __H264SINK__H_
#define __H264SINK__H_

#include "Sink.h"

class H264Sink : public Sink
{
public:
	H264Sink(UsageEnvironment* env, Source* source);
	virtual ~H264Sink();
	static H264Sink* createNew(UsageEnvironment* env, Source* source);

	virtual std::string getMediaDescription(uint16_t port) override;
	virtual std::string getAttribute() override;
	virtual void sendFrame(MediaFrame* frame) override;

private:
	RTPPacket _packet;
	int _clockRate;
	int _fps;
};

#endif // 