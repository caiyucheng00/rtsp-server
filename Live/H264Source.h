#ifndef __H264SOURCE__H_
#define __H264SOURCE__H_

#include <string>
#include "Source.h"

class H264Source : public Source
{
public:
	H264Source(UsageEnvironment* env, const std::string& file);
	virtual ~H264Source();
	static H264Source* createNew(UsageEnvironment* env, const std::string& file);

protected:
	virtual void handleTask() override;

private:
	int getFrameFromH264File(uint8_t* buf, int size);
	FILE* _file;
};

#endif