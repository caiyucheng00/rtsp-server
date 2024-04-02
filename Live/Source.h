#ifndef __SOURCE__H_
#define __SOURCE__H_

#include <queue>
#include <mutex>
#include <stdint.h>
#include "../Scheduler/UsageEnvironment.h"
#include "../Scheduler/ThreadPool.h"

#define FRAME_MAX_SIZE (1024*200)
#define DEFAULT_FRAME_NUM 4

class MediaFrame
{
public:
	MediaFrame() :
		_temp(new uint8_t[FRAME_MAX_SIZE]),
		_buf(nullptr),
		_size(0)
	{

	}
	~MediaFrame() {
		delete[] _temp;
	}

	uint8_t* _temp;
	uint8_t* _buf;
	int _size;

};
class Source
{
public:
	explicit Source(UsageEnvironment* env);
	virtual ~Source();

	MediaFrame* getFrameFromOutputQueue();//从输出队列获取帧
	void putFrameToInputQueue(MediaFrame* frame); // 把帧送入输入队列
	int getFPS() const;
	std::string getSourceName();

protected:
	virtual void handleTask() = 0;
	void setFPS(int fps);

	UsageEnvironment* _env;
	MediaFrame _frame_list[DEFAULT_FRAME_NUM];
	std::queue<MediaFrame*> _frame_input_queue;
	std::queue<MediaFrame*> _frame_output_queue;
	std::mutex _mutex;

	Task _task;
	int _fps;
	std::string _sourceName;

private:
	static void taskCallback(void* arg);
};

#endif