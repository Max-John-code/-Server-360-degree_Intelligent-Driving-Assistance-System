#pragma once
#include "CBaseTask.h"
#include<unistd.h>
#include "protocol.h"
#include<iostream>
#include<map>
#define PACKET_SIZE 2048    //业务包大小&共享内存一个数据区大小
#define SEND_TO_BACK 1      //业务类型：接收客户端业务包
#define GET_FROM_BACK 2     //业务类型：接收后置服务器返回包
using namespace std;
class CChildTask :
    public CBaseTask
{
public:
    CChildTask(char* data, int type, map<int, int>* client, map<int, int>* heart);
    ~CChildTask();

    // 通过 CBaseTask 继承
    virtual void working() override;
    //业务处理函数
    void toBackServer();        //业务包丢给共享内存，后置服务器接收
    void toClient();            //从后置服务器接收返回包，返回包丢给客户端

    void setClient(map<int, int>* client);
    void setHeart(map<int, int>* heart);

    uint32_t crc32(const char* data, size_t size);

private:
    int index;              //记录当前要读取数据区对应的索引区下标
    int businessType;       //记录当前要执行的业务类型
    map<int, int>* cilentMap; //客户端在线map  fd，业务数
    map<int, int>* heartBeatMap;//fd,计次  次数为2 下线
    map<int, int>::iterator iter;//迭代器
    REBUILD_HEAD rsvHead;   //记录从共享内存数据区读到的业务返回包头
};

