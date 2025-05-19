#include<stdio.h>
#include <iostream>
#include <sqlite3.h>
#include "CThreadPool.h"
#include "CChildTask.h"
#include "IPC.h"
#include <fstream>
#include "DBBusiness.h"
#include<map>
#include"CImage.h"
using namespace std;
void* write_log(void* arg);//写日志线程函数
int main()
{
	CThreadPool* threadPool = new CThreadPool();

	IPC* ipc = new IPC();
	//map<用户名,map<文件名,文件信息类>>
	map<string, map<string,CImage*>> imagMap;//最外层map
	//开启写日志线程
	//pthread_t id;
	//pthread_create(&id, NULL, write_log, NULL);

	while (true)
	{
		//等待前置业务包消息
		ipc->getShm();

		//创建从前置接收业务包任务
		CBaseTask* task = new CChildTask("");

		//任务设置共享内存
		task->setIPC(ipc);
		task->setUseridMap(&imagMap);
		//线程池添加子任务
		threadPool->pushTask(task);
	}
	/*
	sqlite3* mydb;
	int res = sqlite3_open("server.db", &mydb);
	if (res != SQLITE_OK)
	{
		perror("sqlite3_open error");
	}
	else
	{
		cout << "sqlite3_open success" << endl;
	}
	*/
	return 0;
}

//写日志线程函数
void* write_log(void* arg)
{
	time_t nowtime;
	struct tm* p;
	int preday = 0;
	int year = 0, mon = 0, day = 0, hour = 0, min = 0, second = 0;//时间
	int res = 0;
	char dirName[20] = { 0 };
	char timeArr[20] = { 0 };//时间
	char fileName[20] = { 0 };//日志文件名
	char filePath[40] = { 0 };//日志文件完整路径
	char sql[200] = { 0 };
	char** qres = nullptr;//总行 包括表头
	int row = 0, col = 0;
	ofstream outfile;//输出文件流
	if (access("./logs", F_OK) == -1)
	{
		res = mkdir("./logs", 0777);
		cout << "创建./logs文件夹 res=" << res << endl;
	}
	while (1)
	{
		time(&nowtime); //获取1970年1月1日0点0分0秒到现在经过的秒数 
		p = localtime(&nowtime); //将秒数转换为本地时间,年从1900算起,需要+1900,月为0-11,所以要+1 
		year = p->tm_year + 1900;
		mon = p->tm_mon + 1;
		day = p->tm_mday;
		hour = p->tm_hour;
		min = p->tm_min;
		second = p->tm_sec;
		if (preday != day)//天数对不上，新一天到了 新建文件夹
		{
			//重组文件夹名
			bzero(dirName, sizeof(dirName));
			sprintf(dirName, "./logs/%04d-%02d-%02d", year, mon, day);
			if (access(dirName, F_OK) == -1)
			{
				res = mkdir(dirName, 0777);
				cout << "创建" << dirName << "文件夹 res=" << res << endl;
				preday = day;
			}
		}
		//重组时间
		bzero(timeArr, sizeof(timeArr));
		sprintf(timeArr, "%04d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, second);
		//重组文件名
		bzero(fileName, sizeof(fileName));
		sprintf(fileName, "log%04d-%02d-%02d %02d:%02d:%02d .txt", year, mon, day, hour, min, second);
		//重组完整文件路径
		bzero(filePath, sizeof(filePath));
		sprintf(filePath, "%s/%s", dirName, fileName);

		sleep(60);//延时时间


		//拼接sql 找到用户名称
		char sql[200] = { 0 };
		sprintf(sql, "SELECT  log_time, log_func, log_type, user_id, log_info\
			FROM tb_log;");
		//WHERE log_time > '%s', timeArr
		//cout << sql << endl; 
		//执行sql

		cout << endl << "读取日志写入文件" << endl;
		res = SingleDB::getInstance()->doSelect(sql, qres, row, col);//获取
		if (0 == res)
		{
			if (row > 0)
			{
				cout << "查到数据往文件写内容" << endl;
				outfile.open(filePath, ios::out);//打开文件流
				for (int i = 1; i < row + 1; i++)
				{
					outfile << "时间：" << qres[i * col] << endl;
					outfile << "功能：" << qres[i * col + 1] << endl;
					outfile << "类型：" << qres[i * col + 2] << endl;
					outfile << "用户id：" << qres[i * col + 3] << endl;
					outfile << "业务：" << qres[i * col + 4] << endl << endl;
				}
				outfile.close();
			}
			else
			{
				cout << "查不到数据，不写" << endl;
			}

		}
		else
		{
			cout << "写日志sql错误" << endl;
		}

	}

	return NULL;
}