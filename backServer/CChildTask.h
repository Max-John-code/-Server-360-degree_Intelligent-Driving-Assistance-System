#pragma once
#include<iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <string>
#include <time.h>
#include <list>
#include <iterator>
#include <algorithm>
#include "protocol.h"
#include "DBBusiness.h"
#include "CBaseTask.h"
#include"CImage.h"
#define PACKET_SIZE 2048    //业务包大小&共享内存一个数据区大小

using namespace std;
class CChildTask :
    public CBaseTask
{
public:
    CChildTask(char* data);
    ~CChildTask();

    // 通过 CBaseTask 继承
    virtual void working() override;
    //业务处理函数
    void loginBusiness();       //登录业务
    void registBusiness();      //注册业务

    void photoUploadBusiness(); //图片零碎数据上传业务
    void pictureUPBusiness(); //图片数据上传数据库业务
    void CImageSpliceTask();//图片拼合业务
    void photoSearchBusiness(); //图片信息检索业务

    void videoUploadBusiness(); //视频数据上传业务
    void videoSearchBusiness(); //视频信息检索业务
    void videoScheduleUpde(); //视频进度条更新业务
    void videoTurnPage();//视频翻页业务

    void addLogBusiness(string work_name, string log_type, int user_id, string log_info);   //全部日志写入业务  

    void getTime(char buffer[]);                    //获取当前时间
    bool createDirs(const std::string& dirName);    //创建新保存图片多级目录

    //公共接口
    void setIndex(int val);

    
private:
    int index;                  //记录当前要读取数据区对应的索引区下标
    char operationPath[80];     //记录当前操作的本地路径
    
    REBUILD_HEAD rsvHead;       //记录从共享内存数据区读到的业务包头
};

