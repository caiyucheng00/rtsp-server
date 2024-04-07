#include "InetAddress.h"

Ipv4Address::Ipv4Address()
{

}

Ipv4Address::Ipv4Address(std::string ip, uint16_t port) :
	_ip(ip),
	_port(port)
{
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	_addr.sin_port = htons(port);
}

void Ipv4Address::setAddr(std::string ip, uint16_t port)
{
	_ip = ip;
	_port = port;
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	_addr.sin_port = htons(port);
}

std::string Ipv4Address::getIp()
{
	return _ip;
}

uint16_t Ipv4Address::getPort()
{
	return _port;
}

struct sockaddr* Ipv4Address::getAddr()
{
	return (struct sockaddr*)&_addr;
}
