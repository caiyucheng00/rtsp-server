#ifndef __SOCKETSOPS__H_
#define __SOCKETSOPS__H_

#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

namespace sockets
{
	int createUdpSock();//默认创建非阻塞的udp描述符
	int createTcpSock();//默认创建非阻塞的tcp描述符
	bool bind(int sockfd, std::string ip, uint16_t port);
	void setReuseAddr(int sockfd, int on);
	bool listen(int sockfd, int backlog);
	int accept(int sockfd);
	int sendto(int sockfd, const void* buf, int len, const struct sockaddr* destAddr); // udp 写入
	int write(int sockfd, const void* buf, int size);// tcp 写入
	void close(int sockfd);
	std::string getLocalIp();
};

#endif