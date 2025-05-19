#pragma once
#include "CBaseSocket.h"
#include "CHostAddress.h"
#include <unistd.h>
#include<iostream>
using namespace std;

class CTcpServer :
    public CBaseSocket
{
public:
    CTcpServer(unsigned short port, int type, int sin_family = AF_INET, int protocol = 0);
    ~CTcpServer();

    // 通过 CBaseSocket 继承
    virtual void stopConnect() override;

    virtual void work() override;

    CHostAddress* getAddress();
    void setAddress(CHostAddress* address);
private:
    CHostAddress* address;
};

