#ifndef __SOCKETSOPS__H_
#define __SOCKETSOPS__H_

#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

namespace sockets
{
	int createUdpSock();//默认创建非阻塞的udp描述符
	int sendto(int sockfd, const void* buf, int len, const struct sockaddr* destAddr); // udp 写入
	int write(int sockfd, const void* buf, int size);// tcp 写入
	void close(int sockfd);
};

#endif