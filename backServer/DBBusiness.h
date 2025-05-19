#pragma once
#include "SingleDB.h"
#include <string.h>
#include<time.h>
//数据库交互单例：执行数据库业务
class DBBusiness
{
public:
	static DBBusiness* getInstance();

	//数据库业务
	int searchUserId(char**& qres, int& row, int& col, const char userId[], const char userPwd[]);	//获取用户ID，判断登录是否成功，返回用户昵称
	int addNewUser(const char userId[], const char userName[], const char userPwd[]);				//用户注册业务

	int addNewPhoto(const char userId[], const char imgPath[], const char imgServerPath[], const char imgTime[], int imgAllBytes);				//图片数据上传业务
	int searchPhoto(char**& qres, int& row, int& col, const char userId[], const int pageIndex);	//图片信息检索业务：检索一页数据

	int updateVideoSchedule(const char account[], const char videoTime[], const int videoCur);//视频进度更新

	int addNewVideo(const char userId[], const char videoPath[], const char videoSaveTime[],		//视频数据上传业务
		const int videoCurFrame, const int videoTotalFrame, const char videoCoverPath[]);
	int searchVideo(char**& qres, int& row, int& col, const char userId[], const int pageIndex);	//视频信息检索业务

	int addNewCar(const char userId[]);

	//log_type  接收  发送
	void writeLogToDb(string work_name, string log_type, string user_id, string log_info);//日志写入数据库
private:
	DBBusiness();
	static DBBusiness* dbBusiness;
};

