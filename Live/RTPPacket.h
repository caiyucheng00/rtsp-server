#ifndef __RTPPACKET__H_
#define __RTPPACKET__H_

#include <stdint.h>
#include <stdlib.h>

#define RTP_VESION              2

#define RTP_PAYLOAD_TYPE_H264   96
#define RTP_PAYLOAD_TYPE_AAC    97

#define RTP_HEADER_SIZE         12
#define RTP_MAX_PKT_SIZE        1400
/*
  *    0                   1                   2                   3
  *    7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0
  *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  *   |V=2|P|X|  CC   |M|     PT      |       sequence number         |
  *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  *   |                           timestamp                           |
  *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  *   |           synchronization source (SSRC) identifier            |
  *   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
  *   |            contributing source (CSRC) identifiers             |
  *   :                             ....                              :
  *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  *
  */
struct RtpHeader {       // 按照后n位
	// byte 0
	uint8_t csrcLen : 4;// CSRC计数器，指示CSRC标识符个数
	uint8_t extension : 1; // 1表明RTP报头后有扩展报头
	uint8_t padding : 1;// 1表明报文尾部填充
	uint8_t version : 2;// RTP协议版本号 2

	// byte 1
	uint8_t payloadType : 7; // 有效载荷类型
	uint8_t marker : 1;  // 视频：一帧结束 音频：会话开始

	// bytes 2,3
	uint16_t seq;       //自增序列号：检测丢失/重排

	// bytes 4-7
	uint32_t timestamp; //时间戳

	// bytes 8-11
	uint32_t ssrc;    //标识同步信号源

	// data
	uint8_t payload[0];
};

struct RtcpHeader
{
	// byte 0
	uint8_t rc : 5;// reception report count
	uint8_t padding : 1;
	uint8_t version : 2;
	// byte 1
	uint8_t packetType;

	// bytes 2,3
	uint16_t length;
};

class RTPPacket
{
public:
	RTPPacket();
	~RTPPacket();

	uint8_t* _buf; // 4+rtpHeader+rtpBody
	uint8_t* _buf4;// rtpHeader+rtpBody
	RtpHeader* const _rtpHeader;
	int _size;// rtpHeader+rtpBody

};

void parseRtpHeader(uint8_t* buf, struct RtpHeader* rtpHeader);
void parseRtcpHeader(uint8_t* buf, struct RtcpHeader* rtcpHeader);
#endif