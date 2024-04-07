#ifndef __INETADDRESS_H_
#define __INETADDRESS_H_

#include <string>
#include <stdint.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

class Ipv4Address
{
public:
	Ipv4Address();
	Ipv4Address(std::string ip, uint16_t port);

	void setAddr(std::string ip, uint16_t port);
	std::string getIp();
	uint16_t getPort();
	struct sockaddr* getAddr();

private:
	std::string _ip;
	uint16_t _port;
	struct sockaddr_in _addr;
};

#endif 