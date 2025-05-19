#include "CHeartBeatTask.h"
CHeartBeatTask::CHeartBeatTask(char* data, int fd, map<int, int>* heartBeatMap) :CBaseTask(data)
{
	this->fd = fd;
	this->heartBeatMap = heartBeatMap;
}

CHeartBeatTask::~CHeartBeatTask()
{
}

void CHeartBeatTask::working()
{
	map<int, int>::iterator iter;
	iter = this->heartBeatMap->find(fd);
	iter->second = 0;
	cout << endl << "┌--------------------------心跳-------------------------┐" << endl;
	cout << "收到fd=" << fd << "心跳，计次归" << iter->second << endl;
	//int res = write(this->fd, this->data, sizeof(HEAD));//发送响应
}