#ifndef __MEDIASESSION__H_
#define __MEDIASESSION__H_

#include <string>
#include <list>
#include "RTPInstance.h"
#include "Sink.h"

#define MEDIA_MAX_TRACK_NUM 2

struct Track
{
	Sink* _sink;
	int _trackId;
	bool _isAlive;
	std::list<RTPInstance*> _rtpInstance_list;

};

enum TrackId
{
	TrackIdNone = -1,
	TrackId0 = 0,
	TrackId1 = 1,
};

class MediaSession
{
public:
	explicit MediaSession(const std::string& sessionName);
	~MediaSession();
	static MediaSession* createNew(std::string sessionName);

	bool addSink(TrackId trackId, Sink* sink);// 添加数据生产者
	bool addRtpInstance(TrackId trackId, RTPInstance* rtpInstance);// 添加数据消费者
	bool removeRtpInstance(RTPInstance* rtpInstance);// 删除数据消费者

	std::string getName() const;
	std::string generateSDPDescription();
	bool startMulticast();
	bool isStartMulticast();
	std::string getMulticastDestAddr() const;
	uint16_t getMulticastDestRtpPort(TrackId trackId);

private:
	Track* getTrack(TrackId trackId);

	static void sendPacketCallback(void* arg1, void* arg2, void* packet, PacketType packetType);
	void handleSendRtpPacket(Track* tarck, RTPPacket* rtpPacket);

	std::string _sessionName;
	std::string _sdp;
	Track _tracks[MEDIA_MAX_TRACK_NUM];
	bool _isStartMulticast;
	std::string _multicastAddr;
	RTPInstance* _multicastRtpInstances[MEDIA_MAX_TRACK_NUM];
	RtcpInstance* _multicastRtcpInstances[MEDIA_MAX_TRACK_NUM];
};

#endif