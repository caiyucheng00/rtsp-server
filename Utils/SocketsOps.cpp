#include "SocketsOps.h"
#include "../Utils/Log.h"

int sockets::createUdpSock()
{
	int sockfd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	unsigned long ul = 1;
	int ret = ioctlsocket(sockfd, FIONBIO, (unsigned long*)&ul);

	if (ret == SOCKET_ERROR) {
		LOGE("SOCKET_ERROR");
	}

	return sockfd;
}

int sockets::createTcpSock()
{
	int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	unsigned long ul = 1;
	int ret = ioctlsocket(sockfd, FIONBIO, (unsigned long*)&ul);  // 非阻塞

	if (ret == SOCKET_ERROR) {
		LOGE("SOCKET_ERROR");
	}

	return sockfd;
}

bool sockets::bind(int sockfd, std::string ip, uint16_t port)
{
	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);

	if (::bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		LOGE("::bind error,fd=%d,ip=%s,port=%d", sockfd, ip.c_str(), port);
		return false;
	}
	return true;
}

void sockets::setReuseAddr(int sockfd, int on)
{
	int optval = on ? 1 : 0;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
}

bool sockets::listen(int sockfd, int backlog)
{
	if (::listen(sockfd, backlog) < 0) {
		LOGE("::listen error,fd=%d,backlog=%d", sockfd, backlog);
		return false;
	}
	return true;
}

int sockets::accept(int sockfd)
{
	struct sockaddr_in addr = { 0 };
	socklen_t addrlen = sizeof(struct sockaddr_in);

	int connfd = ::accept(sockfd, (struct sockaddr*)&addr, &addrlen);

	return connfd;
}

int sockets::sendto(int sockfd, const void* buf, int len, const struct sockaddr* destAddr)
{
	socklen_t addrLen = sizeof(struct sockaddr);
	return ::sendto(sockfd, (char*)buf, len, 0, destAddr, addrLen);
}

int sockets::write(int sockfd, const void* buf, int size)
{
	return ::send(sockfd, (char*)buf, size, 0); 
}

void sockets::close(int sockfd)
{
	::closesocket(sockfd);
}

std::string sockets::getLocalIp()
{
	return "0.0.0.0";
}

