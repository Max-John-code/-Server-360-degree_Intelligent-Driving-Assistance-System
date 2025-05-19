#pragma once
#include<iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include<stdio.h>
#include<unistd.h>
#include <sys/shm.h>
#include<string.h>
#include <sys/msg.h>
#include <map>
#include "protocol.h"

#define LOG_NUM 12 //日志数量
#define BLOCK_NUM 100		//共享内存数据块数量
#define PACKET_SIZE 2048	//业务包大小&共享内存一个数据区大小

//前后置服务器通信类
using namespace std;

typedef struct messagegbuf {
	long mtype;       /* message type, must be > 0 */
	char mtext[50];    /* message data */
}MSGBUF;

union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds* buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short* array;  /* Array for GETALL, SETALL */
	struct seminfo* __buf;  /* Buffer for IPC_INFO
								(Linux-specific) */
};

//实时日志结构体
typedef struct curLog
{
	int businessNum;	//用户执行业务数
	int videoListNum;	//视频列表获取数
	int videoRecordNum;	//视频播放记录业务数
}CURLOG;


class IPC
{
public:
	IPC();
	~IPC();
	//创建/访问信号量
	int sem_create(key_t key, int num_sems);

	//信号量初始化
	void sem_setVal(int sem_id, int sem_num, int value);

	void sem_p(int sem_id, int sem_index);	//信号量P操作	-1加锁
	void sem_v(int sem_id, int sem_index);	//信号量V操作	+1解锁

	//访问共享内存
	void setShm(char resbuf[]);	//业务包丢进共享内存
	void getShm();				//从共享内存获取业务返回包

	map<string, CURLOG> currentLog;	//存储实时日志信息

	//公共接口
	char* getRsvPacket();
	//打印日志
	void printfLog(int allNum);//打印实时日志

	/*
	* 函数名称：ctlCount
	* 函数作用：对指定区块进行++ 或 --
	* 函数参数：参数一：int index  操作下标
						0接受包数  1发送包数
						2用户登入数
						3请求登录 4 请求注册 5请求下线
						6上传视频数据  7上传视频记录  8查询视频数据
						9上传零碎文件  10上传特征图片  11查询图片数据
	*			参数二：int mode 操作模式 0 ++ ，1 --
	* 函数返回：
	*/
	void ctlCount(int index, int mode);

private:
	int shmID, msgID, semID;			//共享内存，消息队列，信号量ID

	int shmArr[BLOCK_NUM];				//共享内存索引区	0—对应下标数据区空闲 1—对应下标数据区有数据
	int sndIndex, rsvIndex;				//记录当前索引区下标
	void* shmAddr;						//共享内存地址

	char sndPacket[PACKET_SIZE];		//接收线程池发来的客户端业务包，转发给共享内存
	char rsvPacket[PACKET_SIZE];		//从共享内存接收业务返回包

	MSGBUF buf;							//接收后置服务器发来的消息
	//业务数组 
	int workNum[12];
	int shmid2;//共享内存id2 
	int semid2;//信号量id2
};

