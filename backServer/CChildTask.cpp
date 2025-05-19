#include "CChildTask.h"

CChildTask::CChildTask(char* data)
	:CBaseTask(data)
{
	this->index = 0;
	//this->operationPath[80] = { 0 };
	//this->rsvHead = { 0 };
	bzero(&(this->rsvHead), sizeof(REBUILD_HEAD));
	bzero(this->operationPath, sizeof(this->operationPath));
}

CChildTask::~CChildTask()
{
	delete this->ipc;
}

void CChildTask::working()
{
	cout << this->data << "正在执行....." << endl;
	//读头
	this->rsvHead = { 0 };														//数据初始化
	memcpy(&this->rsvHead, this->ipc->getRsvPacket(), sizeof(REBUILD_HEAD));
	this->workfd = rsvHead.acceptfd;											//记录当前执行业务客户端fd
	cout << "------------------------------------------------" << endl;
	cout << "后置服务器读取业务包头" << endl;
	cout << "rsvHead.length = " << rsvHead.length << endl;
	cout << "rsvHead.type = " << rsvHead.type << endl;
	cout << "rsvHead.acceptfd = " << rsvHead.acceptfd << endl;

	//读体
	//根据头信息判断业务类型，执行不同的业务逻辑
	switch (rsvHead.type)
	{
	case USER_REGISTER:
		this->registBusiness();
		break;
	case USER_LOGIN:
		this->loginBusiness();
		break;
	case VIDEO_UP_INFO:
		this->videoUploadBusiness();
		break;
	case VIDEO_SCHEDULE_UPDE:
		this->videoScheduleUpde();
		break;
	case VIDEO_TURN_PAGE:
		this->videoTurnPage();
		break;
	case IMAGE_UP_INFO:
		this->photoUploadBusiness();
		break;
	case IMAGE_UP_FINISH:
		this->CImageSpliceTask();
		break;
	case PHOTO_ASK:
		this->pictureUPBusiness();
		break;
	case PHOTO_QUERT:
		this->photoSearchBusiness();
		break;
	}
	cout << "------------------------------------------------" << endl;
}

void CChildTask::loginBusiness()
{
	cout << "------------客户端登录请求业务------------" << endl;
	//实时日志更新 往共享内存塞入数据
	this->ipc->ctlCount(3, 0);//+请求登录
	//登录协议体 接收信息
	LOGIN body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "body.id = " << body.id << endl;
	cout << "body.password = " << body.password << endl;
	cout << "------------------------------------------------" << endl;

	//访问数据库执行对应业务
	LOGIN_BACK loginBackPack = { 0 };	//返回包体

	char** qres = NULL;					//必要参数
	int row = 0, col = 0;
	//数据库寻找用户是否存在
	int res = DBBusiness::getInstance()->searchUserId(qres, row, col, body.id, body.password);
	if (1 == res)
	{
		cout << "客户端登录成功" << endl;
		loginBackPack.result = 1;	//封装业务返回包体  返回登录的结果 1 成功 -1 失败
		memcpy(&loginBackPack.name, qres[1], sizeof(qres[0]));
		cout << "返回用户昵称 loginBackPack.name = " << loginBackPack.name << endl;
		this->ipc->ctlCount(2, 0);//+用户登入
		//获取视频数据
		char** videoQres = NULL;					//必要参数
		int videoRow = 0, videoCol = 0;
		int res = DBBusiness::getInstance()->searchVideo(videoQres, videoRow, videoCol, body.id, 0);
		if (res >= 0)
		{
			cout << "登录视频数据获取成功" << endl;

			//计算必要数据信息
			int videoNum = res;				//该用户拥有的视频总个数
			int pageNum = 0;					//视频总页数
			if (videoNum % 6 == 0)
			{
				pageNum = videoNum / 6;
			}
			else
			{
				pageNum = (videoNum / 6) + 1;
			}
			cout << "用户视频数据信息" << endl;
			cout << "视频总个数 videoNum = " << videoNum << endl;
			cout << "视频总页数 pageNum = " << pageNum << endl;
			cout << "获取的视频数据" << endl;
			loginBackPack.page = videoNum;
			string info = "发送个人结构体数据包：\n用户账号：";
			info += body.id;
			info += "\n密码：";
			info += body.password;
			cout << "准备写入日志表" << endl;
			DBBusiness::getInstance()->writeLogToDb("登录", "接收", body.id, info); //登录 接收 写入日志
			//写日志
			DBBusiness::getInstance()->writeLogToDb("登录", "发送", body.id, info);//登录 发送 写入日志
		}
		else
		{
			cout << "登录视频数据获取失败" << endl;
		}
	}
	else
	{
		cout << "客户端登录失败" << endl;
		loginBackPack.result = -1;	//封装业务返回包体
	}

	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	//memcpy(sndHead.id, body.id, sizeof(body.id));
	sndHead.length = sizeof(loginBackPack);
	sndHead.type = USERLOGIN_BACK;
	//sndHead.CRC = 1;
	sndHead.acceptfd = this->workfd;

	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));							//添加头
	memcpy(sndPacket + sizeof(REBUILD_HEAD), &loginBackPack, sndHead.length);	//添加体

	//返回包丢进共享内存
	this->ipc->setShm(sndPacket);
	cout << "------------------------------------------------" << endl;
}

void CChildTask::registBusiness()
{
	this->ipc->ctlCount(4, 0);//+请求注册
	cout << "------------客户端注册请求业务------------" << endl;

	REG body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "reg.id = " << body.id << endl;
	cout << "reg.name = " << body.name << endl;
	cout << "reg.password = " << body.password << endl;
	cout << "------------------------------------------------" << endl;

	//访问数据库执行对应业务
	TBACK registBackPack = { 0 };

	int res = DBBusiness::getInstance()->addNewUser(body.id, body.name, body.password);
	if (1 == res)
	{
		cout << "客户端注册成功" << endl;
		string info = "发送个人结构体数据包：\n用户名：";
		info += body.name;
		info += "\n账号：";
		info += body.id;
		info += "\n密码：";
		info += body.password;
		cout << "注册业务准备向数据库写入日志数据" << endl;
		DBBusiness::getInstance()->writeLogToDb("注册", "接收", body.id, info); //注册 接收 写入日志 
		DBBusiness::getInstance()->writeLogToDb("注册", "发送", body.id, info); //注册 发送 写入日志
		registBackPack.result = 1;	//封装业务返回包体
		int res1 = DBBusiness::getInstance()->addNewCar(body.id);
		if (res1 == 1)
		{
			cout << "添加车辆成功!!!" << endl;
		}
	}
	else if (0 == res)
	{
		cout << "客户端注册失败，用户Id已存在" << endl;
		registBackPack.result = -1;	//封装业务返回包体
	}
	else if (-1 == res)
	{
		cout << "客户端注册失败，sql语句执行失败" << endl;
		registBackPack.result = -1;	//封装业务返回包体
	}

	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	//memcpy(sndHead.id, body.id, sizeof(body.id));
	sndHead.length = sizeof(registBackPack);
	sndHead.type = BACK_BACK;
	//sndHead.CRC = 1;
	sndHead.acceptfd = this->workfd;

	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));							//添加头
	memcpy(sndPacket + sizeof(REBUILD_HEAD), &registBackPack, sndHead.length);	//添加体

	//返回包丢进共享内存
	this->ipc->setShm(sndPacket);


	cout << "------------------------------------------------" << endl;
}

void CChildTask::photoUploadBusiness()
{
	cout << "------------客户端图片上传请求业务------------" << endl;
	this->ipc->ctlCount(9, 0);//+零碎文件
	FILEINFO imaget = { 0 };//图片结构体
	memcpy(&imaget, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "imaget.fileindex= " << imaget.fileindex << " imaget.account=" << imaget.account << endl;
	//map<用户名,map<文件名,文件信息类>>
	map<string, map<string, CImage*>>::iterator iter;
	iter = imagInfoMap->find(imaget.account);
	//判断该用户是否存在
	if (iter == imagInfoMap->end())
	{
		//cout << "没有这个key" << endl;
		//插入数据
		map<string, CImage*> tem;
		tem.insert(make_pair(imaget.filename,
			new CImage(imaget.fileLegth, imaget.fileNum, imaget.account, imaget.filename, imaget.filepath)));
		this->imagInfoMap->insert(make_pair(imaget.account,tem));
		iter = imagInfoMap->find(imaget.account);
		char tmpchar[20] = { 0 };
		string info = "发送特征图片结构体数据包：\n图片名：";
		info += imaget.filename;
		info += "\n图片大小：";
		sprintf(tmpchar, "%d", imaget.fileLegth);
		info += tmpchar;
		info += "\n图片路径：";
		info += imaget.filepath;
		DBBusiness::getInstance()->writeLogToDb("上传特征图片", "接收", imaget.account, info); //上传特征图片 接收 写入日志
	}
	else
	{
		//判断该用户下是否有这个文件正在上传
		map<string, CImage*>::iterator iter2;
		iter2 = iter->second.find(imaget.filename);
		if (iter2 == iter->second.end())
		{
			//无这个文件在上传
			//插入数据
			map<string, CImage*> tem;
			tem.insert(make_pair(imaget.filename,
				new CImage(imaget.fileLegth, imaget.fileNum, imaget.account, imaget.filename, imaget.filepath)));
			this->imagInfoMap->insert(make_pair(imaget.account, tem));
			iter = imagInfoMap->find(imaget.account);
			char tmpchar[20] = { 0 };
			string info = "发送特征图片结构体数据包：\n图片名：";
			info += imaget.filename;
			info += "\n图片大小：";
			sprintf(tmpchar, "%d", imaget.fileLegth);
			info += tmpchar;
			info += "\n图片路径：";
			info += imaget.filepath;
			DBBusiness::getInstance()->writeLogToDb("上传特征图片", "接收", imaget.account, info); //上传特征图片 接收 写入日志
		}
	}
	int i = iter->second.find(imaget.filename)->second->getAllnums();
	cout << endl << "┌-----------------------上传零碎文件--------------------┐" << endl;
	cout << "图片名：" << imaget.filename << "--第" << imaget.fileindex << "号包--总数" << imaget.length << endl;
	//cout << "allnums= " << i << endl;
	char* imgdata = new char[1000];
	memcpy(imgdata, imaget.context, 1000);
	iter->second.find(imaget.filename)->second->getImg_dataMap().insert(make_pair(imaget.fileindex, imgdata));
	iter->second.find(imaget.filename)->second->addBytes(imaget.length);
	//cout << "size= " << iter->second->getImg_dataMap().size() << endl;

}

void CChildTask::pictureUPBusiness()
{
	this->ipc->ctlCount(10, 0);//+图片信息输入
	cout << "------------客户端图片信息上传数据库请求业务------------" << endl;
	PHOTO body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "保存的用户account = " << body.user_account << endl;
	cout << "保存图片的路径 = " << body.photoPath << endl;
	cout << "保存图片的时间 = " << body.traitPhotoTime << endl;
	cout << "保存图片的总字节 = " << body.allBytes << endl;
	cout << "保存图片的名字 = " << body.photoName << endl;
	cout << "------------------------------------------------" << endl;
	//访问数据库执行对应业务
	string serverPath = "./image/";
	serverPath += body.user_account;
	serverPath += "/";
	serverPath += body.photoName;
	cout << "服务器保存图片路径: " << serverPath << endl;
	string info;//信息
	TBACK photoBackPack = { 0 };
	int res = DBBusiness::getInstance()->addNewPhoto(body.user_account, body.photoPath, serverPath.c_str(),
		body.traitPhotoTime, body.allBytes);
	string khd = body.photoPath;
	khd += +"/";
	khd += body.photoName;
	if (1 == res)
	{
		cout << "客户端保存视频成功" << endl;
		photoBackPack.result = 1;	//封装业务返回包体
		info = "发送视频结构体数据包：\n用户：";
		info += body.user_account;
		info += "\n图片名称：";
		info += body.photoName;
		info += "\n图片客户端路径：";
		info += khd;
		info += "\n图片服务器路径：";
		info += serverPath;
		info += "\n图片总字节：";
		info += to_string(body.allBytes);
		cout << "写入图片成功" << endl;
	}
	else if (0 == res)
	{
		cout << "客户端保存图片失败，图片已存在" << endl;
		photoBackPack.result = -1;	//封装业务返回包体
		info = "写入图片失败";
		cout << "写入图片失败" << endl;
	}
	else if (-1 == res)
	{
		cout << "客户端保存图片失败，sql语句执行失败" << endl;
		photoBackPack.result = -1;	//封装业务返回包体
		info = "写入图片失败";
		cout << "写入图片失败" << endl;
	}

	DBBusiness::getInstance()->writeLogToDb("上传图片数据", "接收", body.user_account, info); //上传图片数据 接收 写入日志

	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	sndHead.length = sizeof(photoBackPack);
	sndHead.type = PHOTOINFO_SAVE_BACK;
	sndHead.acceptfd = this->workfd;

	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));							//添加头
	memcpy(sndPacket + sizeof(REBUILD_HEAD), &photoBackPack, sndHead.length);	//添加体

	//返回包丢进共享内存
	this->ipc->setShm(sndPacket);
	DBBusiness::getInstance()->writeLogToDb("上传图片数据", "发送", body.user_account, info); //上传图片数据 发送 写入日志
	cout << "------------------------------------------------" << endl;
}

void CChildTask::CImageSpliceTask()
{
	this->ipc->ctlCount(10, 0);//+拼合文件
	FILE_FINISH imageokt = { 0 };
	memcpy(&imageokt, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);

	//cout<<" imageokt.user_id= " << imageokt.user_id << " imageokt.img_num=" 
	//	<< imageokt.img_name << " imageokt.img_type=" << imageokt.img_type << endl;
	//map<string, CImage*>::iterator iter; //最外层 useridmap 迭代器
	map<string, map<string, CImage*>>::iterator iter;
	iter = imagInfoMap->find(imageokt.account);
	cout << endl << "┌-----------------------拼合图片------------------------┐" << endl;
	cout << "零碎文件数目" << iter->second.find(imageokt.filename)->second->getImg_dataMap().size() << endl;


	//拼合协议
	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	sndHead.length = sizeof(IMAGEBACKT);
	sndHead.type = IMAGE_UP_BACK;
	sndHead.acceptfd = this->workfd;
	FILE_LOSS imageback = { 0 };//返回包
	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));							//添加头

	if (iter->second.find(imageokt.filename)->second->compareBytesAndNum())
	{
		cout << "校验通过" << endl;
		umask(0);
		int res = 0;
		if (access("./image", F_OK) == -1)
		{
			res = mkdir("./image", 0777);
			cout << "创建./image文件夹 res=" << res << endl;
		}
		string imagepath = "./image/";
		imagepath += imageokt.account;//用户账号
		if (access(imagepath.c_str(), F_OK) == -1)
		{
			res = mkdir(imagepath.c_str(), 0777);
			cout << "创建" << imagepath << " res=" << res << endl;
		}

		string imagepath2;

		// 根据文件名 确定文件要保存到的路径
		imagepath2 = imagepath;
		imagepath2 += "/";
		imagepath2 += iter->second.find(imageokt.filename)->second->getImg_name();
		cout << "imagepath=" << imagepath2 << endl;
		//最后一份的大小
		int endbyte = iter->second.find(imageokt.filename)->second->getAllBytes() % 1000;
		int nums = iter->second.find(imageokt.filename)->second->getAllnums();
		int i = 1;
		cout << "filepath=" << imagepath2 << " endbyte=" << endbyte << endl;
		map<int, char*>::iterator iter2;//文件数据map迭代器
		int wfd = open(imagepath2.c_str(), O_CREAT | O_WRONLY, 0777);
		for (iter2 = iter->second.find(imageokt.filename)->second->getImg_dataMap().begin(); iter2 != iter->second.find(imageokt.filename)->second->getImg_dataMap().end(); iter2++)
		{
			if (i == nums)
			{
				res = write(wfd, iter2->second, endbyte);
				cout << "key=" << iter2->first << " 拼完了关文件" << i << " res=" << res << endl;
			}
			else
			{
				res = write(wfd, iter2->second, 1000);
				cout << "key=" << iter2->first << " 拼合第" << i << "个文件" << " res=" << res << endl;
			}
			++i;
			delete[] iter2->second;//释放图片数据
		}
		close(wfd);

		cout << "校验通过写入日志和数据库，拼合完毕" << endl;
		iter->second.find(imageokt.filename)->second->getWriteLog();
		//cout << "删map" << endl;
		delete iter->second.find(imageokt.filename)->second;
		this->imagInfoMap->erase(imageokt.account);
		//cout << "拼完了" << endl;
		imageback.fileCount = 0;//拼合成功
		//memcpy(buf + sizeof(HEAD), &imageback, head.length);
		memcpy(sndPacket + sizeof(REBUILD_HEAD), &imageback, sndHead.length);	//添加体
		//返回包丢进共享内存
		this->ipc->setShm(sndPacket);
	}
	else
	{
		int index = 0;//对比下标
		cout << "校验失败，客户端需要重发：" << endl;
		map<int, char*>::iterator iter2;//文件数据map迭代器
		//临时存储所有缺少的文件下标
		list<int> indexList;
		string lose_nums;//丢失序列
		char lose_nums2[20] = { 0 };
		for (iter2 = iter->second.find(imageokt.filename)->second->getImg_dataMap().begin(); iter2 != iter->second.find(imageokt.filename)->second->getImg_dataMap().end(); iter2++)
		{
			while (iter2->first != index)
			{
				cout << "中间丢包" << endl;
				cout << "用户" << imageokt.account << ", 第" << index << "号文件包丢失" << endl;
				sprintf(lose_nums2, "%d,", index);
				lose_nums += lose_nums2;
				indexList.push_back(index);
				//imageback.fileCount++;//数据包丢失个数
				++index;
			}
			++index;
		}
		if (iter->second.find(imageokt.filename)->second->getImg_dataMap().size() < iter->second.find(imageokt.filename)->second->getAllnums())
		{
			for (int j = iter->second.find(imageokt.filename)->second->getImg_dataMap().size(); j < iter->second.find(imageokt.filename)->second->getAllnums(); j++)
			{
				cout << "末尾丢包,中间都没有丢" << endl;
				cout << "用户" << imageokt.account << ", 第" << j << "号文件包丢失" << endl;
				sprintf(lose_nums2, "%d,", j);
				lose_nums += lose_nums2;
				indexList.push_back(j);
			}
		}
		

		cout << lose_nums << endl;
		cout << "缺少的大小: lose_nums.size()= " << lose_nums.size() << endl;
		cout << "img_name" << imageokt.filename << endl;
		//计算需要发送的协议总个数
		int counts = 0;
		if (lose_nums.size() % 999 == 0)
		{
			counts = lose_nums.size() / 999;
			if (counts == 0)
			{
				counts = 1;
			}
		}
		else
		{
			counts = lose_nums.size() / 999 + 1;
		}
		int sum = indexList.size();
		cout << "丢失的个数 sum = " << sum << endl;
		int send = 1;
		cout << "需要发送的协议总数: " << counts << endl;
		imageback.count = counts;
		//当前发送的协议个数
		index = 1;
		lose_nums.clear();
		for (list<int>::iterator it5 = indexList.begin(); it5 != indexList.end(); ++it5)
		{
			//拼接发送的序号
			sprintf(lose_nums2, "%d,", (*it5));
			lose_nums += lose_nums2;
			cout << "拼接的内容: lose_nums= " << lose_nums << endl;
			if ((lose_nums.size() >= 995 && lose_nums.size()!= 0) || send == sum)
			{
				strcpy(imageback.filename, iter->second.find(imageokt.filename)->second->getImg_name().c_str());
				strcpy(imageback.buf, lose_nums.c_str());
				imageback.cur = index;
				imageback.fileCount = lose_nums.size();
				memcpy(sndPacket + sizeof(REBUILD_HEAD), &imageback, sndHead.length);	//添加体
				//返回包丢进共享内存
				this->ipc->setShm(sndPacket);
				lose_nums.clear();
				bzero(imageback.filename,sizeof(imageback.filename));
				bzero(imageback.buf, sizeof(imageback.buf));
				index++;
				break;
			}
			send++;
			cout << "send = " << send << endl;
		}
	}
}

void CChildTask::photoSearchBusiness()
{
	this->ipc->ctlCount(11, 0);//+查询图片
	cout << "------------客户端图片翻页请求业务------------" << endl;
	TURN_PICTURE body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "翻页的用户account = " << body.user_acount << endl;
	cout << "当前视频列表的页数 = " << body.page << endl;
	cout << "------------------------------------------------" << endl;
	cout << "==============================接收图片请求写入日志表============================" << endl;
	string info = "接收图片查询请求数据包：\n用户：";
	info += body.user_acount;
	info += "\n查询页码：";
	info += to_string(body.page);
	DBBusiness::getInstance()->writeLogToDb("查询图片信息", "接收", body.user_acount, info); //接收 写入日志
	cout << "==============================================================================" << endl;
	cout << endl << "┌------------------------查询图片-----------------------┐" << endl;
	//执行sql
	char** qres = nullptr;//总行 包括表头
	int row = 0, col = 0;
	GET_PHOTOS_INFO photoBackPack = { 0 };
	int res = DBBusiness::getInstance()->searchPhoto(qres, row, col, body.user_acount, body.page);//获取
	if (res >= 0)
	{
		cout << "客户端获取翻页图片成功" << endl;
		photoBackPack.page = row;	//封装业务返回包体
		photoBackPack.allPhoto = res;//视频总个数
		cout << "==============================图片请求数据库操作写入日志表============================" << endl;
		info = "发回图片查询结果数据包：\n用户：";
		info += body.user_acount;
		info += "\n查询页码：";
		info += to_string(body.page + 1);
		info += "\n查出图片数：";
		info += to_string(row);
		DBBusiness::getInstance()->writeLogToDb("查询图片信息", "发送", body.user_acount, info); //发送 写入日志
		cout << "==============================================================================" << endl;
		for (int i = 1; i <= row; i++)
		{
			cout << qres[i * col] << " " << qres[i * col + 1] << " " << qres[i * col + 2] << endl;

			//视频数据加入返回包体
			memcpy(photoBackPack.photoArr[i - 1].photoPath, qres[i * col], sizeof(photoBackPack.photoArr[i - 1].photoPath));
			memcpy(photoBackPack.photoArr[i - 1].traitPhotoTime , qres[i * col + 1], sizeof(photoBackPack.photoArr[i - 1].traitPhotoTime));
			photoBackPack.photoArr[i - 1].allBytes = atoi(qres[i * col + 2]);
		}
	}
	else if (-1 == res)
	{
		cout << "客户端获取翻页图片失败，sql语句执行失败" << endl;
		photoBackPack.page = -1;	//封装业务返回包体
	}

	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	sndHead.length = sizeof(photoBackPack);
	sndHead.type = PHOTO_QUERT_BACK;
	sndHead.acceptfd = this->workfd;

	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));							//添加头
	memcpy(sndPacket + sizeof(REBUILD_HEAD), &photoBackPack, sndHead.length);	//添加体

	//返回包丢进共享内存
	this->ipc->setShm(sndPacket);
	cout << "------------------------------------------------" << endl;

}

void CChildTask::videoUploadBusiness()
{
	this->ipc->ctlCount(6, 0);//+视频输入
	cout << "------------客户端视频上传请求业务------------" << endl;

	VIDEO_SAVE body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "保存的用户account = " << body.user_account << endl;
	cout << "保存视频的首帧路径 = " << body.video_cover << endl;
	cout << "保存视频的总帧数 = " << body.video_num << endl;
	cout << "保存视频的路径 = " << body.video_path << endl;
	cout << "保存视频的时间 = " << body.video_time << endl;
	cout << "------------------------------------------------" << endl;

	//访问数据库执行对应业务
	string info;//信息
	TBACK videoupBackPack = { 0 };
	int res = DBBusiness::getInstance()->addNewVideo(body.user_account, body.video_path, body.video_time,
		0, body.video_num, body.video_cover);

	if (1 == res)
	{
		cout << "客户端保存视频成功" << endl;
		videoupBackPack.result = 1;	//封装业务返回包体
		char duration[20] = { 0 };//时长
		int sec = body.video_num / 30;//总秒数
		int min = sec / 60;//分钟
		sec = sec % 60;
		sprintf(duration, "%d：%02d", min, sec); //拼合时间
		info = "发送视频结构体数据包：\n用户：";
		info += body.user_account;
		info += "\n视频名称：";
		info += body.video_time;
		info += "\n视频封面路径：";
		info += body.video_cover;
		info += "\n视频路径：";
		info += body.video_path;
		info += "\n视频时长：";
		info += duration;
		cout << "写入视频成功" << endl;
	}
	else if (0 == res)
	{
		cout << "客户端保存视频失败，视频已存在" << endl;
		videoupBackPack.result = -1;	//封装业务返回包体
		info = "写入视频失败";
		cout << "写入视频失败" << endl;
	}
	else if (-1 == res)
	{
		cout << "客户端保存视频失败，sql语句执行失败" << endl;
		videoupBackPack.result = -1;	//封装业务返回包体
		info = "写入视频失败";
		cout << "写入视频失败" << endl;
	}

	DBBusiness::getInstance()->writeLogToDb("上传视频数据", "接收", body.user_account, info); //上传视频数据 接收 写入日志

	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	sndHead.length = sizeof(videoupBackPack);
	sndHead.type = VIDEO_SAVE_BACK;
	sndHead.acceptfd = this->workfd;

	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));							//添加头
	memcpy(sndPacket + sizeof(REBUILD_HEAD), &videoupBackPack, sndHead.length);	//添加体

	//返回包丢进共享内存
	this->ipc->setShm(sndPacket);
	DBBusiness::getInstance()->writeLogToDb("上传视频数据", "发送", body.user_account, info); //上传视频数据 发回 写入日志
	cout << "------------------------------------------------" << endl;

}

void CChildTask::videoSearchBusiness()
{
	cout << "------------客户端视频翻页请求业务------------" << endl;
}

void CChildTask::videoScheduleUpde()
{
	this->ipc->ctlCount(7, 0);//+上传视频记录
	cout << "------------客户端视频进度更新请求业务------------" << endl;
	SCHEDULE_SAVE body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "更新的用户account = " << body.user_acount << endl;
	cout << "更新视频的帧数 = " << body.video_cur << endl;
	cout << "更新视频的时间 = " << body.video_time << endl;
	cout << "------------------------------------------------" << endl;

	//访问数据库执行对应业务
	string info;//信息
	TBACK videoupBackPack = { 0 };
	int res = DBBusiness::getInstance()->updateVideoSchedule(body.user_acount, body.video_time, body.video_cur);
	if (1 == res)
	{
		cout << "客户端更新视频进度成功" << endl;
		videoupBackPack.result = 1;	//封装业务返回包体
		info = "更新视频进度结构体数据包：\n用户：";
		info += body.user_acount;
		info += "\n视频名称：";
		info += body.video_time;
		info += "\n视频更新播放时长：";
		info += to_string(body.video_cur);
	}
	else if (0 == res)
	{
		cout << "客户端更新视频进度" << endl;
		videoupBackPack.result = -1;	//封装业务返回包体
	}
	else if (-1 == res)
	{
		cout << "客户端更新视频进度失败，sql语句执行失败" << endl;
		videoupBackPack.result = -1;	//封装业务返回包体
	}

	DBBusiness::getInstance()->writeLogToDb("更新视频进度", "接收", body.user_acount, info); //更新视频进度 接收 写入日志

	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	sndHead.length = sizeof(videoupBackPack);
	sndHead.type = VIDEO_SCHEDULE_BACK;
	sndHead.acceptfd = this->workfd;

	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));							//添加头
	memcpy(sndPacket + sizeof(REBUILD_HEAD), &videoupBackPack, sndHead.length);	//添加体

	//返回包丢进共享内存
	this->ipc->setShm(sndPacket);

	DBBusiness::getInstance()->writeLogToDb("更新视频进度", "发送", body.user_acount, info); //更新视频进度 发送 写入日志
	cout << "------------------------------------------------" << endl;

}

void CChildTask::videoTurnPage()
{
	this->ipc->ctlCount(8, 0);//+视频数据
	cout << "------------客户端视频翻页请求业务------------" << endl;
	TURN_PAGE body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "翻页的用户account = " << body.user_acount << endl;
	cout << "当前视频列表的页数 = " << body.page << endl;
	cout << "------------------------------------------------" << endl;
	cout << "==============================接收视频请求写入日志表============================" << endl;
	string info = "接收视频查询请求数据包：\n用户：";
	info += body.user_acount;
	info += "\n查询页码：";
	info += to_string(body.page+1);
	DBBusiness::getInstance()->writeLogToDb("查询视频信息", "接收", body.user_acount, info); //接收 写入日志
	cout << "==============================================================================" << endl;
																					 //获取视频数据
	char** videoQres = NULL;					//必要参数
	int videoRow = 0, videoCol = 0;
	//访问数据库执行对应业务
	BACK_PAGE videoupBackPack = { 0 };
	int res = DBBusiness::getInstance()->searchVideo(videoQres, videoRow, videoCol,body.user_acount, body.page);
	if (res >= 0)
	{
		cout << "客户端获取翻页视频成功" << endl;
		videoupBackPack.flag = videoRow;	//封装业务返回包体
		videoupBackPack.video_count = res;//视频总个数
		cout << "==============================视频请求数据库操作写入日志表============================" << endl;
		info = "发回视频查询结果数据包：\n用户：";
		info += body.user_acount;
		info += "\n查询页码：";
		info += to_string(body.page+1);
		info += "\n查出视频数：";
		info += to_string(videoRow);
		DBBusiness::getInstance()->writeLogToDb("查询视频信息", "发送", body.user_acount, info); //发送 写入日志
		cout << "==============================================================================" << endl;
		for (int i = 1; i <= videoRow; i++)
		{
			cout << videoQres[i * videoCol] << " " << videoQres[i * videoCol + 1] << " " << videoQres[i * videoCol + 2] << " " << videoQres[i * videoCol + 3]
				<< " " << videoQres[i * videoCol + 4] << endl;

			//视频数据加入返回包体
			memcpy(videoupBackPack.videoArr[i - 1].videoPath, videoQres[i * videoCol], sizeof(videoupBackPack.videoArr[i - 1].videoPath));
			videoupBackPack.videoArr[i - 1].curFrame = atoi(videoQres[i * videoCol + 1]);
			videoupBackPack.videoArr[i - 1].totalFrame = atoi(videoQres[i * videoCol + 2]);
			memcpy(videoupBackPack.videoArr[i - 1].videoCoverPath, videoQres[i * videoCol + 3], sizeof(videoupBackPack.videoArr[i - 1].videoCoverPath));
			memcpy(videoupBackPack.videoArr[i - 1].videoTime, videoQres[i * videoCol + 4], sizeof(videoupBackPack.videoArr[i - 1].videoTime));
		}
	}
	else if (-1 == res)
	{
		cout << "客户端获取翻页视频失败，sql语句执行失败" << endl;
		videoupBackPack.flag = -1;	//封装业务返回包体
	}

	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	sndHead.length = sizeof(videoupBackPack);
	sndHead.type = VIDEO_TURN_PAGE_BCAK;
	sndHead.acceptfd = this->workfd;

	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));							//添加头
	memcpy(sndPacket + sizeof(REBUILD_HEAD), &videoupBackPack, sndHead.length);	//添加体

	//返回包丢进共享内存
	this->ipc->setShm(sndPacket);
	cout << "------------------------------------------------" << endl;
}

void CChildTask::addLogBusiness(string work_name, string log_type, int user_id, string log_info)
{
	char logTime[20] = { 0 };
	time_t nowtime;
	time(&nowtime); //获取1970年1月1日0点0分0秒到现在经过的秒数 
	tm* p = localtime(&nowtime); //将秒数转换为本地时间,年从1900算起,需要+1900,月为0-11,所以要+1 
	sprintf(logTime, "%04d-%02d-%02d %02d:%02d:%02d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

	//拼接sql 找到用户名称
	char sql[500] = { 0 };
	//执行sql
	char** qres = nullptr;//总行 包括表头
	int row = 0, col = 0;
	int res = -1;
	//拼接sql 插入新日志
	sprintf(sql, "INSERT INTO tbl_log(log_time, work_id, log_type, user_id, log_info) "
		"VALUES('%s',%d, %d, %d, '%s');", logTime, work_name, log_type, user_id, log_info.c_str());
	//cout << sql << endl;
	//执行sql
	//res = DBBusiness::getInstance()->insertDelUpd(sql);//插入新日志
	cout << "---插入了日志记录---" << endl;
	/*cout << "时间：" << logTime<< endl;
	cout << "功能：" << workid << endl;
	cout << "类型：" << log_type << endl;
	cout << "用户 id：" << userid << endl;
	cout << log_info << endl<<endl;*/
}

void CChildTask::getTime(char buffer[])
{
	//获取当前时间
	time_t rawtime;		//time_t 能够把系统时间和日期表示为某种整数
	struct tm* info;	//结构类型 tm 把日期和时间以 C 结构的形式保存

	time(&rawtime);										//返回系统的当前日历时间
	info = localtime(&rawtime);							//返回一个指向表示本地时间的 tm 结构的指针
	strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", info);	//格式化日期和时间为指定的格式
}

bool CChildTask::createDirs(const std::string& dirName)
{
	return false;
}

void CChildTask::setIndex(int val)
{
	this->index = val;
}




