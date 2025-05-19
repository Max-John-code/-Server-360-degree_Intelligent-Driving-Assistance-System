#include "CTcpServer.h"

CTcpServer::CTcpServer(unsigned short port, int type, int sin_family, int protocol)
	:CBaseSocket(type, sin_family, protocol)
{
	this->address = new CHostAddress(port);
}

CTcpServer::~CTcpServer()
{
	delete this->address;
}

void CTcpServer::stopConnect()
{
	if (this->socketfd != 0 && this->socketfd > 0)
	{
		close(this->socketfd);
		this->setSocketfd(0);
	}
}

void CTcpServer::work()
{
	int opt_val = 1;
	//设置套接字选项的值
	setsockopt(this->socketfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&opt_val, sizeof(opt_val));

	if (bind(this->socketfd, this->address->getAddr(), this->address->getLength()) == -1)
	{
		perror("bind error");
	}
	//监听这个文件描述符,是否有客户端来连接
	if (listen(this->socketfd, 10) == -1)
	{
		perror("listen error");
	}

	cout << "服务器网络搭建成功" << endl;

}

CHostAddress* CTcpServer::getAddress()
{
	return this->address;
}

void CTcpServer::setAddress(CHostAddress* address)
{
	this->address = address;
}
