#include "CBaseSocket.h"

CBaseSocket::CBaseSocket(int type, int sin_family, int protocol)
{
    this->type = type;
    startConnect(sin_family, protocol);

}

CBaseSocket::~CBaseSocket()
{
}

int CBaseSocket::getSocketfd()
{
    return this->socketfd;
}

void CBaseSocket::setSocketfd(int socketfd)
{
    this->socketfd = socketfd;
}

void CBaseSocket::startConnect(int sin_family, int protocol)
{
    this->socketfd = socket(sin_family, this->type, protocol);
    if (this->socketfd < 0)
    {
        perror("socketfd error");
    }
}
