#include "DBBusiness.h"

DBBusiness* DBBusiness::dbBusiness = NULL;
DBBusiness* DBBusiness::getInstance()
{
	if (NULL == DBBusiness::dbBusiness)
	{
		DBBusiness::dbBusiness = new DBBusiness();
	}
	return DBBusiness::dbBusiness;
}

int DBBusiness::searchUserId(char**& qres, int& row, int& col, const char userId[], const char userPwd[])
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：获取用户ID，判断登录是否成功，返回用户昵称" << endl;

    //拼接sql语句
    char sqlCode[] = "select user_account from tb_user where user_account='%s' and user_pwd='%s';";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, userId, userPwd);

    //必要参数
    char** judgeQres = nullptr;
    int judgeRow = 0, judgeCol = 0;

    //执行sql语句
    int res = SingleDB::getInstance()->doSelect(newSqlCode, judgeQres, judgeRow, judgeCol);
    cout << "getUserNameAndPwd res = " << res << endl;
    if (0 == res)
    {
        //获取用户昵称
        //拼接sql语句
        char userNameSqlCode[] = "select user_name from tb_user where user_account='%s' and user_pwd='%s';";
        char newUserNameSqlCode[strlen(userNameSqlCode)];
        sprintf(newUserNameSqlCode, userNameSqlCode, userId, userPwd);

        //执行sql语句
        int rres = SingleDB::getInstance()->doSelect(newUserNameSqlCode, qres, row, col);
        if (rres == 0)
        {
            return judgeRow; //1：客户端登录成功；0：客户端登录失败
        }
        else
        {
            return -1;  //sql语句执行失败
        }
    }
    else
    {
        return -1;  //sql语句执行失败
    }
}

int DBBusiness::addNewUser(const char userId[], const char userName[], const char userPwd[])
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：客户端注册业务" << endl;

    //拼接sql语句
    char sqlCode[] = "select user_account from tb_user where user_account='%s';";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, userId);

    //必要参数
    char** qres = nullptr;
    int row = 0, col = 0;

    //执行sql语句
    int judgeRes = SingleDB::getInstance()->doSelect(newSqlCode, qres, row, col);
    cout << "addNewUser judgeRes = " << judgeRes << endl;
    if (0 == row)   //1：用户Id已存在，注册失败；0：用户Id不存在，进行注册业务
    {
        //注册业务
        //拼接sql语句
        char registSqlCode[] = "insert into tb_user(user_account,user_name,user_pwd) values('%s','%s','%s');";
        char newRegistSqlCode[strlen(registSqlCode)];
        sprintf(newRegistSqlCode, registSqlCode, userId, userName, userPwd);

        //必要参数
        char** qres = nullptr;
        int row = 0, col = 0;

        //执行sql语句
        int res = SingleDB::getInstance()->doupdateInsertDel(newRegistSqlCode);
        cout << "addNewUser res = " << res << endl;
        if (0 == res)
        {
            return 1;   //注册成功
        }
        else
        {
            return -1;  //sql执行语句失败
        }
    }
    else
    {
        return -1;   //注册失败
    }

}
int DBBusiness::addNewPhoto(const char userId[], const char imgPath[], const char imgServerPath[], const char imgTime[], int imgAllBytes)
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：添加图片信息业务" << endl;
    //必要参数
    char** qres = nullptr;
    int row = 0, col = 0;
    int res = -1;
    char sql[] = "select user_id from tb_user where user_account='%s';";
    char newsql[strlen(sql)];
    //拼接sql语句
    sprintf(newsql, sql, userId);
    res = SingleDB::getInstance()->doSelect(newsql, qres, row, col);
    int user_id = atoi(qres[col]);

    //执行sql语句
    char sqlCode[] = "select * from tb_image where user_id=%d and img_path='%s'";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, user_id, imgPath);
    res = SingleDB::getInstance()->doSelect(newSqlCode, qres, row, col);
    cout << "addNewVideo res = " << res << endl;
    if (0 == res)
    {
        //视频信息不存在，做插入
        if (0 == row)
        {
            //拼接sql语句
            char insertSqlCode[] = "insert into tb_image(img_path,img_serverpath,img_time,img_allbytes,user_id) values('%s','%s','%s',%d,%d);";
            char insertNewSqlCode[strlen(insertSqlCode) * 2];
            sprintf(insertNewSqlCode, insertSqlCode, imgPath, imgServerPath, imgTime, imgAllBytes, user_id);

            //必要参数
            char** insertQres = nullptr;
            int insertRow = 0, insertCol = 0;

            //执行sql语句
            int res = SingleDB::getInstance()->doupdateInsertDel(insertNewSqlCode);
            cout << "addNewVideo res = " << res << endl;
            if (0 == res)
            {
                return 1;   //图片信息添加成功
            }
            else
            {
                return -1;  //sql语句执行失败
            }
        }
    }
    else
    {
        return -1;  //sql语句执行失败
    }
}
int DBBusiness::searchPhoto(char**& qres, int& row, int& col, const char userId[], const int pageIndex)
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：客户端图片信息检索业务" << endl;
    //必要参数
    char** qres1 = nullptr;
    int row1 = 0, col1 = 0;
    int res = -1;
    char sql[] = "select user_id from tb_user where user_account='%s';";
    char newsql[strlen(sql)];
    //拼接sql语句
    sprintf(newsql, sql, userId);
    res = SingleDB::getInstance()->doSelect(newsql, qres1, row1, col1);
    int user_id = atoi(qres1[col1]);

    //拼接sql语句
    char sqlCode[] = "select img_path,img_time,img_allbytes from tb_image where user_id=%d ORDER BY img_id DESC limit 9 offset 9*%d;";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, user_id, pageIndex);

    //执行sql语句
    res = SingleDB::getInstance()->doSelect(newSqlCode, qres, row, col);
    cout << "searchPhoto res = " << res << endl;
    if (0 == res)
    {
        cout << "翻页的图片准备就绪!!!" << endl;
        //返回用户所有视频数据个数
        //拼接sql语句
        char sqlCode2[] = "select * from tb_image where user_id=%d";
        char newSqlCode2[strlen(sqlCode2)];
        sprintf(newSqlCode2, sqlCode2, user_id);

        //必要参数
        char** qres2 = nullptr;
        int row2 = 0, col2 = 0;

        //执行sql语句
        int rres = SingleDB::getInstance()->doSelect(newSqlCode2, qres2, row2, col2);
        cout << "searchPhoto res = " << res << endl;
        if (0 == rres)
        {
            cout << "翻页的用户的图片总数: " << row2 << endl;
            return row2; //0：未检索到视频信息；其他：检索到视频信息
        }
        else
        {
            return -1;  //sql语句执行失败
        }
    }
    else
    {
        return -1;  //sql语句执行失败
    }
}
//-1 sql错误 1 成功
int DBBusiness::updateVideoSchedule(const char account[], const char videoTime[], const int videoCur)
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：更新视频信息业务" << endl;
    //必要参数
    char** qres = nullptr;
    int row = 0, col = 0;
    int res = -1;
    int user_id = 0;
    char sql[] = "select user_id from tb_user where user_account='%s';";
    char newsql[strlen(sql)];
    //拼接sql语句
    sprintf(newsql, sql, account);
    res = SingleDB::getInstance()->doSelect(newsql, qres, row, col);
    if (res == 0)
    {
        if (row > 0)
        {
            cout << "数据库找到账号对应的用户id: " << atoi(qres[col]) << endl;
            user_id = atoi(qres[col]);
            //拼接sql语句
            char updateSqlCode[] = "update tb_video set video_cur=%d where user_id=%d and video_time='%s';";
            char updateNewSqlCode[strlen(updateSqlCode) * 2];
            sprintf(updateNewSqlCode, updateSqlCode, videoCur, user_id, videoTime);

            //执行sql语句
            res = SingleDB::getInstance()->doupdateInsertDel(updateNewSqlCode);
            cout << "addNewVideo res = " << res << endl;
            if (0 == res)
            {
                cout << "数据库视频进度更新成功" << endl;
                return 1;   //更新成功
            }
            else
            {
                cout << "数据库视频进度更新sql执行语句失败" << endl;
                return -1;  //sql执行语句失败
            }
        }
        
    }
    else
    {
        return -1;//sql错误
    }

    

    return 0;
}

int DBBusiness::addNewVideo(const char userId[], const char videoPath[], const char videoSaveTime[], const int videoCurFrame, const int videoTotalFrame, const char videoCoverPath[])
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：新增视频信息业务" << endl;
    //必要参数
    char** qres = nullptr;
    int row = 0, col = 0;
    int res = -1;
    char sql[] = "select user_id from tb_user where user_account='%s';";
    char newsql[strlen(sql)];
    //拼接sql语句
    sprintf(newsql, sql, userId);
    res = SingleDB::getInstance()->doSelect(newsql, qres, row, col);
    int user_id = atoi(qres[col]);

    //执行sql语句
    char sqlCode[] = "select * from tb_video where user_id=%d and video_path='%s'";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, user_id, videoPath);
    res = SingleDB::getInstance()->doSelect(newSqlCode, qres, row, col);
    cout << "addNewVideo res = " << res << endl;
    if (0 == res)
    {
        //视频信息不存在，做插入
        if (0 == row)
        {
            //拼接sql语句
            char insertSqlCode[] = "insert into tb_video(video_path,video_cur,video_num,video_cover,video_time,user_id) values('%s',%d,%d,'%s','%s',%d);";
            char insertNewSqlCode[strlen(insertSqlCode) * 2];
            sprintf(insertNewSqlCode, insertSqlCode, videoPath, videoCurFrame, videoTotalFrame, videoCoverPath, videoSaveTime, user_id);

            //必要参数
            char** insertQres = nullptr;
            int insertRow = 0, insertCol = 0;

            //执行sql语句
            int res = SingleDB::getInstance()->doupdateInsertDel(insertNewSqlCode);
            cout << "addNewVideo res = " << res << endl;
            if (0 == res)
            {
                return 1;   //视频信息添加成功
            }
            else
            {
                return -1;  //sql语句执行失败
            }
        }
        //视频信息存在，做更新
        /*else
        {
            //拼接sql语句
            char updateSqlCode[] = "update tbl_videoInfo set videoCurFrame=%d,videoTotalFrame=%d where userId='%s' and videoPath='%s';";
            char updateNewSqlCode[strlen(updateSqlCode) * 2];
            sprintf(updateNewSqlCode, updateSqlCode, videoCurFrame, videoTotalFrame, userId, videoPath);

            //必要参数
            char** updateQres = nullptr;
            int updateRow = 0, updateCol = 0;

            //执行sql语句
            int res = SingleDB::getInstance()->doupdateInsertDel(updateNewSqlCode);
            cout << "addNewVideo res = " << res << endl;
            if (1 == res)
            {
                return 1;   //视频信息添加成功
            }
            else
            {
                return -1;  //sql语句执行失败
            }
        }
        */
    }
    else
    {
        return -1;  //sql语句执行失败
    }
}

int DBBusiness::searchVideo(char**& qres, int& row, int& col, const char userId[], const int pageIndex)
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：客户端视频信息检索业务" << endl;
    //必要参数
    char** queQres = nullptr;
    int queRow = 0, queCol = 0;
    int res = -1;
    char sql[] = "select user_id from tb_user where user_account='%s';";
    char newsql[strlen(sql)];
    //拼接sql语句
    sprintf(newsql, sql, userId);
    res = SingleDB::getInstance()->doSelect(newsql, queQres, queRow, queCol);
    int user_id = atoi(queQres[queCol]);

    //拼接sql语句
    char sqlCode[] = "select video_path,video_cur,video_num,video_cover,video_time from tb_video where user_id=%d ORDER BY video_id DESC limit 6 offset 6*%d;";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, user_id, pageIndex);

    //执行sql语句
    res = SingleDB::getInstance()->doSelect(newSqlCode, qres, row, col);
    cout << "searchVideo res = " << res << endl;
    if (0 == res)
    {
        cout << "翻页的视频准备就绪!!!" << endl;
        //返回用户所有视频数据个数
        //拼接sql语句
        char sqlCode2[] = "select * from tb_video where user_id=%d";
        char newSqlCode2[strlen(sqlCode2)];
        sprintf(newSqlCode2, sqlCode2, user_id);

        //必要参数
        char** qres2 = nullptr;
        int row2 = 0, col2 = 0;

        //执行sql语句
        int rres = SingleDB::getInstance()->doSelect(newSqlCode2, qres2, row2, col2);
        cout << "searchPhoto res = " << res << endl;
        if (0 == rres)
        {
            cout << "翻页的用户的视频总数: " << row2 << endl;
            return row2; //0：未检索到视频信息；其他：检索到视频信息
        }
        else
        {
            return -1;  //sql语句执行失败
        }
    }
    else
    {
        return -1;  //sql语句执行失败
    }
}

int DBBusiness::addNewCar(const char userId[])
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：新增车辆信息业务" << endl;
    //必要参数
    char** qres = nullptr;
    int row = 0, col = 0;
    int res = -1;
    char sql[] = "select user_id from tb_user where user_account='%s';";
    char newsql[strlen(sql)];
    //拼接sql语句
    sprintf(newsql, sql, userId);
    res = SingleDB::getInstance()->doSelect(newsql, qres, row, col);
    int user_id = atoi(qres[col]);
    srand(time(NULL));
    int number = 1000 + rand() % 9999;
    char brand[50] = { 0 };
    sprintf(brand, "闽A%d", number);

    //拼接sql语句
    char sqlCode[] = "insert into tb_car(car_brand,user_id) values('%s',%d);";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, brand, user_id);
    //执行sql语句
    res = SingleDB::getInstance()->doupdateInsertDel(newSqlCode);
    cout << "addNewCar res = " << res << endl;
    if (0 == res)
    {
        return 1;   //添加车辆成功
    }
    else
    {
        return -1;  //sql执行语句失败
    }
}

void DBBusiness::writeLogToDb(string work_name, string log_type, string user_id, string log_info)
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：写入日志表业务" << endl;
    //必要参数
    char** queQres = nullptr;
    int queRow = 0, queCol = 0;
    int res = -1;
    char sql[] = "select user_id from tb_user where user_account='%s';";
    char newsql[strlen(sql)];
    //拼接sql语句
    sprintf(newsql, sql, user_id.c_str());
    res = SingleDB::getInstance()->doSelect(newsql, queQres, queRow, queCol);
    int userID = atoi(queQres[queCol]);
    cout << "找到写入的用户id =" << userID << endl;
    char logTime[20] = { 0 };
    time_t nowtime;
    time(&nowtime); //获取1970年1月1日0点0分0秒到现在经过的秒数 
    tm* p = localtime(&nowtime); //将秒数转换为本地时间,年从1900算起,需要+1900,月为0-11,所以要+1 
    sprintf(logTime, "%04d-%02d-%02d %02d:%02d:%02d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

    //拼接sql 找到用户名称
    char sqlNew[500] = { 0 };
    //执行sql
    char** qres = nullptr;//总行 包括表头
    int row = 0, col = 0;
    //拼接sql 插入新日志
    sprintf(sqlNew, "INSERT INTO tb_log(log_time, log_func, log_type, log_info, user_id) "
        "VALUES('%s','%s', '%s', '%s', %d);", logTime, work_name.c_str(), log_type.c_str(), log_info.c_str(), userID);
    //cout << sql << endl;
    //执行sql
    res = SingleDB::getInstance()->doupdateInsertDel(sqlNew);//插入新日志
    cout << "--------------插入了日志记录--------" << endl;
    cout << "时间：" << logTime<< endl;
    cout << "功能：" << work_name << endl;
    cout << "类型：" << log_type << endl;
    cout << "用户 id：" << user_id << endl;
    cout << log_info << endl<<endl;
    cout << "------------------------------------" << endl;
}

DBBusiness::DBBusiness()
{
}
