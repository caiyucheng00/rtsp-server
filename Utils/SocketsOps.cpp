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
