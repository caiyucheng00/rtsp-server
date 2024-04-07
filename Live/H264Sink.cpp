#include "H264Sink.h"

H264Sink::H264Sink(UsageEnvironment* env, Source* source) :
	Sink(env, source, RTP_PAYLOAD_TYPE_H264),
	_clockRate(90000),
	_fps(source->getFPS())
{
	runEvery(1000 / _fps);    //interval = 40ms
}

H264Sink::~H264Sink() {

}

H264Sink* H264Sink::createNew(UsageEnvironment* env, Source* source)
{
	if (!source) return NULL;

	return new H264Sink(env, source);
}

std::string H264Sink::getMediaDescription(uint16_t port)
{
	char buf[100] = { 0 };
	sprintf(buf, "m=video %hu RTP/AVP %d", port, _payloadType);

	return std::string(buf);
}

std::string H264Sink::getAttribute()
{
	char buf[100];
	sprintf(buf, "a=rtpmap:%d H264/%d\r\n", _payloadType, _clockRate);
	sprintf(buf + strlen(buf), "a=framerate:%d", _fps);

	return std::string(buf);
}

void H264Sink::sendFrame(MediaFrame* frame)
{
	// 发送RTP数据包
	RtpHeader* rtpHeader = _packet._rtpHeader;

	uint8_t naluType = frame->_buf[0];  //第一个字节
	if (frame->_size <= RTP_MAX_PKT_SIZE) {    // 直接发送
		memcpy(rtpHeader->payload, frame->_buf, frame->_size);  // frame填充packet的load
		_packet._size = RTP_HEADER_SIZE + frame->_size;
		sendRTPPacket(&_packet);    //填充头部字段
		_seq++;

		if ((naluType & 0x1F) == 7 || (naluType & 0x1F) == 8) return;   //sps pps 不加时间戳
	}
	else                                       // 分包
	{
		int pktNum = frame->_size / RTP_MAX_PKT_SIZE;       // 有几个完整的包
		int remainPktSize = frame->_size % RTP_MAX_PKT_SIZE; // 剩余不完整包的大小
		int pos = 1;

		// 完整数据包
		for (int i = 0; i < pktNum; ++i) {
			/*
		   *     FU Indicator
		   *    0 1 2 3 4 5 6 7
		   *   +-+-+-+-+-+-+-+-+
		   *   |F|NRI|  Type   |
		   *   +---------------+
		   * */
			rtpHeader->payload[0] = (naluType & 0x60) | 28; //(naluType & 0x60)表示nalu的重要性，28表示为分片

			/*
			*      FU Header
			*    0 1 2 3 4 5 6 7
			*   +-+-+-+-+-+-+-+-+
			*   |S|E|R|  Type   |
			*   +---------------+
			* */
			rtpHeader->payload[1] = naluType & 0x1F;

			if (i == 0) //第一包数据
				rtpHeader->payload[1] |= 0x80; // start
			else if (remainPktSize == 0 && i == pktNum - 1) //最后一包数据
				rtpHeader->payload[1] |= 0x40; // end

			memcpy(rtpHeader->payload + 2, frame->_buf + pos, RTP_MAX_PKT_SIZE);
			_packet._size = RTP_HEADER_SIZE + 2 + RTP_MAX_PKT_SIZE;
			sendRTPPacket(&_packet);

			_seq++;
			pos += RTP_MAX_PKT_SIZE;
		}

		// 剩余
		if (remainPktSize > 0)
		{
			rtpHeader->payload[0] = (naluType & 0x60) | 28;
			rtpHeader->payload[1] = naluType & 0x1F;
			rtpHeader->payload[1] |= 0x40; //end

			memcpy(rtpHeader->payload + 2, frame->_buf + pos, remainPktSize);
			_packet._size = RTP_HEADER_SIZE + 2 + remainPktSize;
			sendRTPPacket(&_packet);

			_seq++;
		}
	}

	_timestamp += _clockRate / _fps;

}
