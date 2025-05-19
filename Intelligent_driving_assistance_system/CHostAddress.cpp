#include "CHostAddress.h"

CHostAddress::CHostAddress(unsigned short port, unsigned int ip)
{
	this->port = port;
	this->ip = ip;
	// 配置服务器地址
	addr.sin_family = AF_INET;
	//服务器系统默认IP地址:对于IPv4: 通配地址由常值INADDR_ANY来指定
	addr.sin_addr.s_addr = this->ip;
	//地址结构的端口地址,网络字节序
	//所谓的端口，就好像是门牌号一样，客户端可以通过ip地址找到对应的服务器端，
	//但是服务器端是有很多端口的，每个应用程序对应一个端口号，
	// 通过类似门牌号的端口号，客户端才能真正的访问到该服务器。
	//为了对端口进行区分，将每个端口进行了编号，这就是端口号
	addr.sin_port = htons(this->port);

	this->length = sizeof(this->addr);

	cout << "this->port = " << this->port << endl;
}

CHostAddress::~CHostAddress()
{
}

unsigned int CHostAddress::getIP()
{
	return this->port;
}

void CHostAddress::setIP(unsigned int ip)
{
	this->ip = ip;
}

unsigned short CHostAddress::getPort()
{
	return this->port;
}

void CHostAddress::setPort(unsigned short port)
{
	this->port = port;
}

sockaddr_in CHostAddress::getAddr_in()
{
	return this->addr;
}

sockaddr* CHostAddress::getAddr()
{
	return (struct sockaddr*)(&this->addr);
}

int CHostAddress::getLength()
{
	return this->length;
}
