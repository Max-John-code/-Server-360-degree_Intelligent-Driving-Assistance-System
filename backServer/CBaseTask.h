#pragma once
#include<string.h>
#include "IPC.h"
#include"CImage.h"
#define PACKET_SIZE 2048
class CBaseTask
{
public:
	CBaseTask(char* data);
	virtual ~CBaseTask();
	virtual void working() = 0;

	//公共接口
	void setWorkfd(int val);
	void setIPC(IPC* ipc);
	void setUseridMap(map<string, map<string, CImage*>>* imgMap);
protected:
	int workfd;					//记录当前连接的客户端fd
	//char比int 节省内存
	char data[PACKET_SIZE];
	map<string, map<string, CImage*>>* imagInfoMap;
	IPC* ipc;					//记录获取的共享内存类对象
};

