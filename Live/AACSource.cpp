#include "AACSource.h"
#include "../Utils/Log.h"

AACSource::AACSource(UsageEnvironment* env, const std::string& file) :
	Source(env)
{
	_sourceName = file;
	_file = fopen(file.c_str(), "rb");
	setFPS(43);

	for (int i = 0; i < DEFAULT_FRAME_NUM; ++i) {
		_env->getThreadPool()->addTask(_task);
	}
}

AACSource::~AACSource()
{
	fclose(_file);
}

AACSource* AACSource::createNew(UsageEnvironment* env, const std::string& file)	
{
	return new AACSource(env, file);
}

void AACSource::handleTask()
{
	std::lock_guard<std::mutex> lck(_mutex);
	if (_frame_input_queue.empty()) {
		return;
	}

	MediaFrame* frame = _frame_input_queue.front();
	frame->_size = getFrameFromAACFile(frame->_temp, FRAME_MAX_SIZE);
	if (frame->_size < 0) {
		return;
	}
	frame->_buf = frame->_temp;

	_frame_input_queue.pop();
	_frame_output_queue.push(frame);
}

int AACSource::getFrameFromAACFile(uint8_t* buf, int size)
{
	if (!_file) return -1;

	uint8_t tmpBuf[7];
	int ret = 0;
	ret = fread(tmpBuf, 1, 7, _file);
	if (ret <= 0) {
		fseek(_file, 0, SEEK_SET);
		ret = fread(tmpBuf, 1, 7, _file);
		if (ret <= 0) return -1;
	}

	if (!parseAdtsHeader(tmpBuf, &_adtsHeader)) return -1;
	if (_adtsHeader.aacFrameLength > size) return -1;

	memcpy(buf, tmpBuf, 7);
	ret = fread(buf + 7, 1, _adtsHeader.aacFrameLength - 7, _file);
	if (ret < 0) {
		LOGE("read error");
		return -1;
	}

	return _adtsHeader.aacFrameLength;
}

bool AACSource::parseAdtsHeader(uint8_t* buf, struct AdtsHeader* res)
{
	memset(res, 0, sizeof(*res));

	if ((buf[0] == 0xFF) && ((buf[1] & 0xF0) == 0xF0))
	{
		res->id = ((unsigned int)buf[1] & 0x08) >> 3;
		res->layer = ((unsigned int)buf[1] & 0x06) >> 1;
		res->protectionAbsent = (unsigned int)buf[1] & 0x01;
		res->profile = ((unsigned int)buf[2] & 0xc0) >> 6;
		res->samplingFreqIndex = ((unsigned int)buf[2] & 0x3c) >> 2;
		res->privateBit = ((unsigned int)buf[2] & 0x02) >> 1;
		res->channelCfg = ((((unsigned int)buf[2] & 0x01) << 2) | (((unsigned int)buf[3] & 0xc0) >> 6));
		res->originalCopy = ((unsigned int)buf[3] & 0x20) >> 5;
		res->home = ((unsigned int)buf[3] & 0x10) >> 4;
		res->copyrightIdentificationBit = ((unsigned int)buf[3] & 0x08) >> 3;
		res->copyrightIdentificationStart = (unsigned int)buf[3] & 0x04 >> 2;
		res->aacFrameLength = (((((unsigned int)buf[3]) & 0x03) << 11) |
			(((unsigned int)buf[4] & 0xFF) << 3) |
			((unsigned int)buf[5] & 0xE0) >> 5);
		res->adtsBufferFullness = (((unsigned int)buf[5] & 0x1f) << 6 |
			((unsigned int)buf[6] & 0xfc) >> 2);
		res->numberOfRawDataBlockInFrame = ((unsigned int)buf[6] & 0x03);

		return true;
	}
	else
	{
		LOGE("failed to parse adts header");
		return false;
	}
}
