#include "AACSink.h"

AACSink::AACSink(UsageEnvironment* env, Source* source) :
	Sink(env, source, RTP_PAYLOAD_TYPE_AAC),
	_sampleRate(44100),
	_channels(2),
	_fps(source->getFPS())
{
	_marker = 1;
	runEvery(1000 / _fps);    //interval = 43ms
}

AACSink::~AACSink() 
{

}

AACSink* AACSink::createNew(UsageEnvironment* env, Source* source)
{
	return new AACSink(env, source);
}

std::string AACSink::getMediaDescription(uint16_t port)
{
	char buf[100] = { 0 };
	sprintf(buf, "m=audio %hu RTP/AVP %d", port, _payloadType);

	return std::string(buf);
}

std::string AACSink::getAttribute()
{
	char buf[500] = { 0 };
	sprintf(buf, "a=rtpmap:97 mpeg4-generic/%u/%u\r\n", _sampleRate, _channels);

	uint8_t index = 0;
	for (index = 0; index < 16; index++)
	{
		if (AACSampleRate[index] == _sampleRate)
			break;
	}
	if (index == 16)
		return "";

	uint8_t profile = 1;
	char configStr[10] = { 0 };
	sprintf(configStr, "%02x%02x", (uint8_t)((profile + 1) << 3) | (index >> 1),
		(uint8_t)((index << 7) | (_channels << 3)));

	sprintf(buf + strlen(buf),
		"a=fmtp:%d profile-level-id=1;"
		"mode=AAC-hbr;"
		"sizelength=13;indexlength=3;indexdeltalength=3;"
		"config=%04u",
		_payloadType,
		atoi(configStr));

	return std::string(buf);
}

void AACSink::sendFrame(MediaFrame* frame)
{
	RtpHeader* rtpHeader = _packet._rtpHeader;
	int frameSize = frame->_size - 7; //去掉aac头部

	rtpHeader->payload[0] = 0x00;
	rtpHeader->payload[1] = 0x10;
	rtpHeader->payload[2] = (frameSize & 0x1FE0) >> 5; //高8位
	rtpHeader->payload[3] = (frameSize & 0x1F) << 3; //低5位

	/* 去掉aac的头部 */
	memcpy(rtpHeader->payload + 4, frame->_buf + 7, frameSize);
	_packet._size = RTP_HEADER_SIZE + 4 + frameSize;

	sendRTPPacket(&_packet);     // 填充packet头部

	_seq++;

	/* (1000 / mFps) 表示一帧多少毫秒 */
	_timestamp += _sampleRate * (1000 / _fps) / 1000;
}
