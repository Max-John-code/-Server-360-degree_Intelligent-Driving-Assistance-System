#include <iostream>
#include "CEpollServer.h"
#include<stdio.h>
using namespace std;

int main()
{
	CEpollServer* epoll = new CEpollServer(10002);
	epoll->EpollWork();

	return 0;
}