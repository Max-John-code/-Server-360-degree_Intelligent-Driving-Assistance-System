#pragma once
//防止重复编译
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include<netinet/in.h>
#include<iostream>
using namespace std;

class CHostAddress
{
public:
	CHostAddress(unsigned short port, unsigned int ip = INADDR_ANY);
	~CHostAddress();

	unsigned int getIP();
	void setIP(unsigned int ip);

	unsigned short getPort();
	void setPort(unsigned short port);

	struct sockaddr_in getAddr_in();
	struct sockaddr* getAddr();

	int getLength();
private:
	//端口 无符号类型 短整型
	unsigned short port;
	//ip地址无符号类型 整型
	unsigned int ip;
	//一个指向特定协议的地址结构的指针
	struct sockaddr_in addr;
	//addr的长度
	int length;
};

