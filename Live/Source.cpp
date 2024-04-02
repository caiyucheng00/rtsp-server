#include "Source.h"
#include "../Utils/Log.h"

Source::Source(UsageEnvironment* env) :
	_env(env),
	_fps(0)
{
	for (int i = 0; i < DEFAULT_FRAME_NUM; ++i) {
		_frame_input_queue.push(&_frame_list[i]);    // 复用
	}

	_task.setTaskCallback(taskCallback, this);         // 实例化时 设置回调
}

Source::~Source()
{
	LOGI("~Source()");
}

MediaFrame* Source::getFrameFromOutputQueue()
{
	std::lock_guard<std::mutex> lck(_mutex);
	if (_frame_output_queue.empty()) {
		return NULL;
	}

	MediaFrame* frame = _frame_output_queue.front();
	_frame_output_queue.pop();

	return frame;
}

void Source::putFrameToInputQueue(MediaFrame* frame)
{
	std::lock_guard<std::mutex> lck(_mutex);
	_frame_input_queue.push(frame);
	_env->getThreadPool()->addTask(_task);      // 添加任务
}

int Source::getFPS() const
{
	return _fps;
}

std::string Source::getSourceName()
{
	return _sourceName;
}

void Source::setFPS(int fps)
{
	_fps = fps;
}

void Source::taskCallback(void* arg)
{
	Source* source = (Source*)arg;
	source->handleTask();
}
