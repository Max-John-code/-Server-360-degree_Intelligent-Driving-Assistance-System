#pragma once
//----------------------------------------------------------业务类型宏定义-----------------------
#define USER_REGISTER 1		//注册（请求）
#define BACK_BACK 2		//反馈包（反馈）
#define USER_LOGIN 3		//登录（请求）
#define USERLOGIN_BACK 4	//登录返回（反馈）
#define VIDEO_UP_INFO 5		//视频信息上传（请求）
#define VIDEO_TURN_PAGE 6   //视频翻页（请求）
#define VIDEO_TURN_PAGE_BCAK 7   //视频翻页（反馈）
#define VIDEO_SCHEDULE_UPDE 8  //视频进度更新
#define VIDEO_SAVE_BACK 9//视频保存返回
#define HEART_BEAT 10 //心跳包
#define VIDEO_SCHEDULE_BACK 11//视频进度更新返回
#define IMAGE_UP_INFO 12//图片和图片信息上传（请求）
#define IMAGE_UP_FINISH 13//图片发送完成确认
#define IMAGE_UP_BACK 14//特征图片上传数据库返回
#define PHOTO_ASK 15 //图片信息保存数据库请求
#define PHOTO_QUERT 16//图片查询请求
#define PHOTO_INFO_BACK 17//图片信息返回包
#define PHOTOINFO_SAVE_BACK 18//图片信息保存返回
#define PHOTO_QUERT_BACK 19//图片查询返回
#define POLYNOMIAL 0x04C11DB7
#define POLY_INIT  0xFFFFFFFF
#define POLY_END   0x00000000
//1 - 注册 2 - 注册返回 3 - 登录 4 - 登陆返回 5 - 视频保存 6 - 视频翻页 
//7 - 视频翻页返回 8 - 视频进度更新 9 - 视频保存返回 10 - 心跳协议 11 - 视频进度更新返回


/*#define PHOTO_ASK_INFO 10   //请求获得图片信息
#define VIDEO_ASK_INFO 12   //请求获取视频信息
#define BACK_BACK 6			//反馈包（反馈）         
#define GET_VIDEO_LIST 7	//视频信息列表反馈包（反馈）
#define GET_IMAGE_LIST 8	//图片信息列表反馈包（反馈）
#define PHOTO_BACK 9        //图片是否上传成功（反馈） ---也指向协议体 6 
#define VIDEO_BACK 11		//视频是否上传成功（反馈） ---也指向协议体 6*/
//1 - 注册 2 - 注册返回 / 视频保存返回 / 特征图片上传数据库返回 / 视频进度更新返回 3 - 登录
//4 - 登陆返回 5 - 视频保存 6 - 视频翻页 7 - 视频翻页返回 8 - 视频进度更新 9 - 特征图片上传数据库 
//10-心跳协议  11-视频保存返回 12-特征图片上传数据库返回 13-视频进度更新返回

//----------------------------------------------------------协议头------------------------------------
//客户端→前置服务器
typedef struct head
{
	int type;		//业务类型
	int length;		//协议体长度
	uint32_t CRC;	//CRC校验码
}HEAD;

//前置服务器→后置服务器
typedef struct rebuiltHead
{
	int type;		//业务类型
	int length;		//协议体长度
	int acceptfd;	//客户端网络连接描述符
}REBUILD_HEAD;

//----------------------------------------------------------协议体------------------------------------
//协议体1
//业务： 客户端登录 → 服务器 
typedef struct login
{
	char id[10];			//用户账号（id）
	char password[50];		//用户密码（MD5加密后的）
}LOGIN;

//协议体2
//业务： 客户端注册 → 服务器 
typedef struct registerr
{
	char id[10];			//用户账号（id）
	char name[10];			//用户昵称
	char password[50];		//用户密码（MD5加密后的）
}REG;

//协议体3
//业务： 视频信息上传 → 服务器
typedef struct videoInfo
{
	char videoPath[50];				//视频存储路径
	char videoCoverPath[50];		//视频封面存储路径
	char videoTime[30];				//视频存储时间 （客户端无需上传，服务器写入数据时记当前时间）
	int curFrame;					//视频播放当前帧数
	int totalFrame;					//视频总帧数
}VIDEO_INFO;

//协议体4
//业务：图片 与 图片信息 上传→服务器
typedef struct photoInfo
{
	char photoPath[50];				//图片存储路径
	char traitPhotoTime[30];		//特征图片存储时间（客户端无需上传，服务器写入数据时记当前时间）
	char p_fileName[60];			//文件名称
	char p_filePageContext[1000];	//文件包内容
	int p_filePageLen;              //文件包长度
	int p_filePageIndex;			//当前文件包下标
	int p_fileSize;					//文件总大小
	int p_filePageNum;				//碎片文件总个数
}PHOTO_INFO;

typedef struct fileinfo
{
	char account[20];//用户名
	char filename[50];//文件名
	char filepath[150];//客户端本地路径
	int fileLegth;//文件总字节
	int fileNum;//碎片文件总数
	int fileindex;//当前碎片文件序号
	int length;//当前碎片文件字节数
	char context[1000]; //文件二进制内容（当前碎片文件）
}PICTURE_INFO;

//协议体16
//图片查询协议体
typedef struct turn_picture
{
	int page; //图片页数，0是切换界面 page*9+1 9
	char user_acount[10];   //用户账号
}TURN_PICTURE;

//协议体10
//业务：特征视频信息请求包
typedef struct  videoAsk
{
	int video_index;
	char id[10];	//用户账号（id）
}VIDEO_ASK;

//视频保存协议体
typedef struct video_save
{
	char video_path[50];//视频路径
	int video_num;//视频总帧数
	char video_cover[50]; // 视频首帧路径
	char video_time[30]; //视频录制时间
	char user_account[10];//用户账号
}VIDEO_SAVE;

//----------------------------------------------------------服务器反馈包→客户端反馈包

//协议体5
//服务器对登录的反馈→客户端
typedef struct loginBack
{
	int result;						//返回结果: 1.成功  -1.失败  0.空（例：服务器没有找到 用户信息/视频信息/图片信息时 给出为空的反馈）
	char name[10];				 //用户昵称
	int page;						//需要的视频信息列表的页数 （按每页6个数据计算） 默认为 第一页的下标
}LOGIN_BACK;

//协议体6
//服务器对请求结果的反馈→客户端
typedef struct backPack
{
	int result;						//返回结果: 1.成功  -1.失败  0.空（例：服务器没有找到 用户信息/视频信息/图片信息时 给出为空的反馈）
}TBACK;

//协议体7
//服务器对视频信息的反馈→客户端 
//视频翻页返回协议体
typedef struct back_page
{
	int flag;// 成功标志 0 - 失败 1 - 成功
	VIDEO_INFO videoArr[6];//视频结构体数组
	int video_count;	//该用户视频总个数
}BACK_PAGE;

//视频进度更新
typedef struct schedule_save
{
	char video_time[30]; //视频录制时间
	int video_cur;	//视频当前帧数
	char user_acount[10];   //用户账号
}SCHEDULE_SAVE;


//协议体8
//服务器对图片信息的反馈→客户端
//图片信息结构体
typedef struct photo
{
	char photoPath[50];			//图片存储路径
	char traitPhotoTime[30];	//特征图片存储时间
	char user_account[10];		//用户账号
}PHOTO;

//图片信息返回包
typedef struct getPhotosInfo
{
	int allPhoto;               //返回图片总个数
	int page;					//需要的图片信息列表的页数 （按每页6个数据计算）//返回总页数，用于边界判断
	PHOTO photoArr[6];			//图片信息结构体数组 默认6个
}GET_PHOTOS_INFO;

//视频翻页协议体
typedef struct turn_page
{
	int page; //视频页数，0是切换界面 page*6+1 6
	char user_acount[10];   //用户账号
}TURN_PAGE;