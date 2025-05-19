#include "CImage.h"
CImage::CImage(int img_allBytes, int img_allNums, string account, string img_name, string img_path)
{
	this->img_allBytes = img_allBytes;
	this->img_allNums = img_allNums;
	this->account = account;
	this->img_name = img_name;
	this->img_path = img_path;
	this->currentBytes = 0;//当前字节为0
}

CImage::~CImage()
{
	this->img_name.clear();
	this->img_path.clear();
	img_dataMap.clear();
	account.clear();
	//cout << "图片走析构" << endl;
}

map<int, char*>& CImage::getImg_dataMap()
{
	// TODO: 在此处插入 return 语句
	return this->img_dataMap;//返回值
}
/*
	* 函数作用：比对文件总字节和当前字节,并且比对传入num和map的size
	* 返回值：相等 true 不相等 false
	*/
bool CImage::compareBytesAndNum()
{
	cout << "总字节= " << this->img_allBytes << " 当前字节=" << this->currentBytes << endl;
	cout << "总文件数=" << this->img_allNums << " 当前文件数=" << this->img_dataMap.size() << endl;
	if (this->img_allBytes == this->currentBytes && this->img_allNums == this->img_dataMap.size())//同时满足字节数相等 且 数量相等
	{
		return true;
	}
	else
	{
		return false;
	}

}
/*
	* 函数名称：addBytes
	* 函数作用：对文件当前总字节数进行累加
	* 函数参数：int bytes 字节
	* 函数返回：
	*/
void CImage::addBytes(int bytes)
{
	this->currentBytes += bytes;
}

int CImage::getAllnums()
{
	return this->img_allNums;
}

int CImage::getAllBytes()
{
	return this->img_allBytes;
}

string CImage::getImg_name()
{
	return this->img_name;
}

string CImage::getImg_path()
{
	return this->img_path;
}

//写入日志
void CImage::getWriteLog()
{
	//数据库添加数据
	/*int res = DBBusiness::getInstance()->addNewPhoto(account.c_str(), img_path.c_str(), img_name.c_str(), img_allBytes);//插入新用户
	if (1 == res)
	{
		cout << "新增图片成功" << endl;
	}
	else
	{
		cout << "新增图片失败" << endl;
	}*/
	char tmpchar[20] = { 0 };
	string khd = img_path+"/";
	khd += img_name;
	string info = "发送特征图片结构体数据包：\n图片名：";
	info += img_name;
	info += "\n图片大小：";
	sprintf(tmpchar, "%d", img_allBytes);
	info += tmpchar;
	info += "\n图片路径：";
	info += khd;
	DBBusiness::getInstance()->writeLogToDb("上传特征图片", "发送", this->account, info); //登录 接收 写入日志
}
