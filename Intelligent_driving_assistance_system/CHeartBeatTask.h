#pragma once
#include<iostream>
#include <unistd.h>
#include "CBaseTask.h"
#include <map>
#include "IPC.h"
#include "protocol.h"
using namespace std;


#include "CBaseTask.h"
class CHeartBeatTask :
    public CBaseTask
{
public:
    CHeartBeatTask(char* data, int fd, map<int, int>* heartBeatMap);
    ~CHeartBeatTask();

    // 通过 CBaseTask 继承
    void working() override;
private:
    map<int, int>* heartBeatMap;
    int fd;
};

