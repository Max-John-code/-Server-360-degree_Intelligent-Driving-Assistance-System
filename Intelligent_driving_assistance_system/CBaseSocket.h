#pragma once
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<iostream>

using namespace std;
class CBaseSocket
{
public:
	CBaseSocket(int type, int sin_family = AF_INET, int protocol = 0);
	virtual ~CBaseSocket();

	int getSocketfd();
	void setSocketfd(int socketfd);
	void startConnect(int sin_family, int protocol);
	virtual void stopConnect() = 0;
	virtual void work() = 0;

	
protected:
	//网络连接类型
	int type;
	int socketfd;//记录当前连接的服务器的文件描述符
	
};

