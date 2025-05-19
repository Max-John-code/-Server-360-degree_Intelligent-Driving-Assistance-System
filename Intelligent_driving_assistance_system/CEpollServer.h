#pragma once
#include<iostream>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include<stdio.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include<string.h>
#include "CTcpServer.h"
#include "CThreadPool.h"
#include "CChildTask.h"
#include "CHeartBeatTask.h"
#include "IPC.h"
#include "protocol.h"
#include<map>
#include<list>
#define EPOLL_SIZE 10
#define PACKET_SIZE 2048
#define HEART_BEAT_TIME 30		//心跳业务测试时间段：30s

//心跳检测结构体
typedef struct heartBeat
{
	int fd;		//客户端连接fd
	int time;	//客户端最后一次操作时间，有操作就加一
}HEARTBEAT;

using namespace std;

//CEpollServer类：接收N个客户端连接fd和客户端发来的业务包（业务包发给线程池）
class CEpollServer
{
public:
	CEpollServer(unsigned short port);
	~CEpollServer();

	//void work();
	void setSocketfd(int fd);
	void EpollControl(int op, int fd, int type);//epoll事件队列添加or删除事件
	void EpollWork();

	void updateOnlineClient(int fd);			//更新在线用户数据，用于心跳检测
	void deleteClient(int fd);					//删除下线用户数据

	static void* epollThreadFun(void* arg);		//epoll线程执行函数：执行接收后置服务器返回包业务
	static void* printfLogsThread(void* arg);		//epoll线程执行函数：执行接收后置服务器返回包业务
	static void* socketOnline(void* arg);//网络连接的用户,但是还没有登录
	static void* heartBeatThreadFun(void* arg);	//心跳检测执行函数：客户端长期不发消息，断开客户端fd
private:
	//epollfd epoll文件描述符
	int epollfd;//epoll文件描述符
	int epollWaitNum;//已经发生的事件个数
	struct epoll_event epollevent;//epoll事件结构体
	//5代表能同时接受同时发生事件fd的数量
	//epolleventArray 是就绪列表
	struct epoll_event epolleventArray[EPOLL_SIZE];//epoll事件就序数组：保存已经触发事件的fd
	CTcpServer* tcp;//epoll类包含tcp类
	int acceptfd;//客户端qcceptfd
	int socketfd;//服务器socketfd
	int res;
	char packet[PACKET_SIZE];//接收客户端发来的业务包
	CThreadPool* pool;//epoll类 包含 线程池类
	IPC* ipc;//epoll类 包含 共享内存类
	int allFdNum;//有效链接数
	//map<int, int> onlineClients;			//在线用户map string：用户；HEARTBEAT：心跳检测结构体
	//map<int, int> onlineClients_heartBeat;	//心跳检测map，用于比较客户端是否存活
	HEAD head;
	map<int, int> heartBeatMap;//fd,计次  次数为3 下线
	map<int, int> cilentMap;//客户端在线map  fd，业务数
	map<int, int> onlineClients;//网络连接用户map:key: fd  value: 计数 1分钟
};

