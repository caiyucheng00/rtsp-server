#include "H264Source.h"
#include "../Utils/Log.h"

static inline int startCode3(uint8_t* buf);
static inline int startCode4(uint8_t* buf);

H264Source::H264Source(UsageEnvironment* env, const std::string& file) :
	Source(env)
{
	_sourceName = file;
	_file = fopen(file.c_str(), "rb");
	setFPS(25);

	for (int i = 0; i < DEFAULT_FRAME_NUM; ++i) {
		_env->getThreadPool()->addTask(_task);     //task执行的是handleTask
	}
}

H264Source::~H264Source()
{
	fclose(_file);
}

H264Source* H264Source::createNew(UsageEnvironment* env, const std::string& file)
{
	return new H264Source(env, file);
}

void H264Source::handleTask()
{
	std::lock_guard<std::mutex> lck(_mutex);
	if (_frame_input_queue.empty()) {
		return;
	}

	MediaFrame* frame = _frame_input_queue.front();    // 获取一个空的Frame
	int startCodeNum = 0;

	while (true) {
		frame->_size = getFrameFromH264File(frame->_temp, FRAME_MAX_SIZE);  //填充Frame
		if (frame->_size < 0) {
			return;
		}

		if (startCode3(frame->_temp)) {
			startCodeNum = 3;
		}
		else
		{
			startCodeNum = 4;
		}
		frame->_buf = frame->_temp + startCodeNum;   //填充Frame不包括startcode
		frame->_size -= startCodeNum;

		uint8_t naluType = frame->_buf[0] & 0x1F;
		if (0x09 == naluType) {
			continue;
		}
		else if (0x07 == naluType || 0x08 == naluType) {
			break;
		}
		else {
			break;
		}

	}

	_frame_input_queue.pop();
	_frame_output_queue.push(frame);
}

static inline int startCode3(uint8_t* buf)
{
	if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
		return 1;
	else
		return 0;
}

static inline int startCode4(uint8_t* buf)
{
	if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
		return 1;
	else
		return 0;
}

static uint8_t* findNextStartCode(uint8_t* buf, int len) {
	if (len < 3) return NULL;

	for (int i = 0; i < len - 3; ++i) {
		if (startCode3(buf) || startCode4(buf)) {
			return buf;
		}

		++buf;
	}

	if (startCode3(buf))
	{
		return buf;
	}

	return NULL;
}

int H264Source::getFrameFromH264File(uint8_t* buf, int size)
{
	if (!_file) return -1;

	int ret = 0;
	int frameSize = 0;
	uint8_t* nextStartCode;

	ret = fread(buf, 1, size, _file);
	if (!startCode3(buf) && !startCode4(buf)) {
		fseek(_file, 0, SEEK_SET);
		LOGE("Read %s error, no startCode3 and no startCode4", _sourceName.c_str());
		return -1;
	}

	nextStartCode = findNextStartCode(buf + 3, ret - 3);    // 寻找下一个startcode
	if (!nextStartCode) {
		fseek(_file, 0, SEEK_SET);
		frameSize = ret;
		LOGE("Read %s error, no nextStartCode, ret=%d", _sourceName.c_str(), ret);
	}
	else {
		frameSize = (nextStartCode - buf);      // 一个NALU的帧长
		fseek(_file, frameSize - ret, SEEK_CUR);
	}

	return frameSize;
}
