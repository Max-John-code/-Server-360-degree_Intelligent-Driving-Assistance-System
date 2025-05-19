#pragma once
#include<iostream>
#include<pthread.h>
#include<list>
#include<queue>
#include<algorithm>
#include "CBaseTask.h"
#define MIN_THREAD_NUM 10
#define MAX_THREAD_NUM 40

using namespace std;
class CThreadPool
{
public:
	// num 线程池的线程默认个数
	CThreadPool(const int num = MIN_THREAD_NUM);
	~CThreadPool();

	//添加任务
	void pushTask(CBaseTask* task);
	//从任务队列取任务
	CBaseTask* popTask();

	//操作忙碌/空闲线程移动
	void moveToIdle(pthread_t id);
	void moveToBusy(pthread_t id);

	//判断任务队列是否为空
	bool queueIsEmpty();

	//操作互斥量
	void lock();
	void unlock();

	//操作条件变量
	void wait();
	void wakeup();

	static void* pthread_function(void* arg);
private:
	int threadMinNum;//线程数量最小值
	int threadMaxNum;//线程数量最大值
	queue<CBaseTask*>taskQueue;//任务队列
	list<pthread_t>idleList;//空闲线程链表
	list<pthread_t>busyList;//忙碌线程链表
	pthread_mutex_t mutex;//互斥量
	pthread_cond_t cond;//线程条件变量 控制 阻塞/唤醒
};

