#include "CChildTask.h"

CChildTask::CChildTask(char* data, int type, map<int, int>* client, map<int, int>* heart)
	:CBaseTask(data)
{
	this->index = 0;
	this->businessType = type;
	//this->rsvHead = { 0 };
	bzero(&(this->rsvHead),sizeof(REBUILD_HEAD));
	this->heartBeatMap = heart;
	this->cilentMap = client;
}

CChildTask::~CChildTask()
{
	delete this->ipc;
}

void CChildTask::working()
{
	cout << this->data << "正在执行....." << endl;
	switch (this->businessType)
	{
	case SEND_TO_BACK:
		this->toBackServer();
		break;
	case GET_FROM_BACK:
		this->toClient();
		break;
	}
}

void CChildTask::toBackServer()
{
	//1.接收客户端发来的业务包
	cout << "前置服务器业务执行中.............." << this->data << endl;

	//读头读体
	HEAD oldHead = { 0 };
	char headBuf[PACKET_SIZE] = { 0 };
	char bodyBuf[PACKET_SIZE] = { 0 };

	memcpy(headBuf, this->data, sizeof(HEAD));
	memcpy(&oldHead, headBuf, sizeof(HEAD));
	cout << "--------------------------------------------------------" << endl;
	cout << "前置服务器读取业务包头" << endl;
	cout << "oldHead.type = " << oldHead.type << endl;
	cout << "oldHead.length = " << oldHead.length << endl;
	cout << "oldHead.CRC = " << oldHead.CRC << endl;
	memcpy(bodyBuf, this->data + sizeof(HEAD), oldHead.length);
	cout << "前置服务器读取业务包体" << endl;

	//CRC校验
	uint32_t crcnum2 = this->crc32(this->data + sizeof(HEAD), oldHead.length);
	cout << "计算出来的CRC = " << crcnum2 << endl;
	if (crcnum2 == oldHead.CRC)
	{
		//CRC校验成功
		//业务包发给后置服务器
		//重新封装头
		REBUILD_HEAD newHead = { 0 };
		newHead.type = oldHead.type;
		newHead.length = oldHead.length;
		newHead.acceptfd = this->workfd;
		cout << "重新封装头" << endl;
		cout << "newHead.type = " << newHead.type << endl;
		cout << "newHead.length = " << newHead.length << endl;
		cout << "newHead.acceptfd = " << newHead.acceptfd << endl;

		//重新封装包
		char newPacket[PACKET_SIZE] = { 0 };
		memcpy(newPacket, &newHead, sizeof(REBUILD_HEAD));															//添加协议头
		memcpy(newPacket + sizeof(REBUILD_HEAD), bodyBuf, newHead.length);											//添加协议体
		cout << "前置服务器新包封装完成 " << endl;

		//新包丢进共享内存
		ipc->setShm(newPacket);

		this->iter = this->cilentMap->find(this->workfd);
		++this->iter->second;//对map值进行++
		this->ipc->ctlCount(0, 0);//接收包数+
		string num = "100";
	}
	else
	{
		//读头读体
		HEAD head = { 0 };
		cout << "CRC校验错误，客户端需重发" << endl;
		TBACK backmag = { 0 };
		backmag.result = 0;//CRC校验错误
		switch (head.type)
		{
		case USER_LOGIN:
			head.type = USERLOGIN_BACK;
			break;
		case USER_REGISTER:
			head.type = BACK_BACK;
			break;
		case VIDEO_UP_INFO:
			head.type = BACK_BACK;
			break;
		case VIDEO_TURN_PAGE:
			head.type = VIDEO_TURN_PAGE_BCAK;
			break;
		case VIDEO_SCHEDULE_UPDE:
			head.type = VIDEO_SCHEDULE_BACK;
			break;
		case IMAGE_UP_INFO:
			return;
		case PHOTO_ASK:
			head.type = PHOTOINFO_SAVE_BACK;
			break;
		case PHOTO_QUERT:
			PHOTO_QUERT_BACK;
			break;
		}
		head.length = sizeof(TBACK);
		bzero(this->data, sizeof(this->data));
		memcpy(this->data, &head, sizeof(HEAD));
		memcpy(this->data + sizeof(HEAD), &backmag, sizeof(TBACK));//最后带上 fd
		int res = write(this->workfd, this->data, sizeof(HEAD) + head.length);
		cout << " 要求客户端重发 res=" << res << endl;
	}
	cout << "------------------------------------------------" << endl;
}

void CChildTask::toClient()
{
	//2.等待后置服务器返回业务返回包
	cout << "前置服务器返回业务执行中.............." << endl;

	//读头读体
	this->rsvHead = { 0 };						//数据初始化
	char returnBodyBuf[PACKET_SIZE] = { 0 };

	memcpy(&this->rsvHead, this->ipc->getRsvPacket(), sizeof(REBUILD_HEAD));
	cout << "------------------------------------------------" << endl;
	cout << "前置服务器读取返回业务包头" << endl;
	cout << "rsvHead.length = " << rsvHead.length << endl;
	cout << "rsvHead.type = " << rsvHead.type << endl;
	cout << "rsvHead.acceptfd = " << rsvHead.acceptfd << endl;

	memcpy(returnBodyBuf, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "前置服务器读取返回业务包体" << endl;

	if (this->rsvHead.type == USERLOGIN_BACK)
	{
		LOGIN_BACK backmsg = { 0 };
		memcpy(&backmsg, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
		if (backmsg.result == 1)
		{
			cout << "心跳添加用户 fd=" << rsvHead.acceptfd << endl;
			this->heartBeatMap->insert(make_pair(rsvHead.acceptfd, 0));
		}
	}
	//测试打印
	/*
	if (this->rsvHead.type == GET_IMAGE_LIST)
	{
		GET_PHOTOS_INFO photoList;
		memcpy(&photoList, returnBodyBuf, this->rsvHead.length);
		cout << "photoList数据打印" << endl;
		cout << photoList.allPhoto << endl;
		cout << photoList.page << endl;
		for (int i = 1; i <= photoList.allPhoto; i++)
		{
			cout << photoList.photoArr[i - 1].photoPath << endl;
			cout << photoList.photoArr[i - 1].traitPhotoTime << endl;
		}
	}*/
	//重新封装返回包头
	HEAD returnHead = { 0 };
	returnHead.type = this->rsvHead.type;
	returnHead.length = this->rsvHead.length;
	cout << "重新封装头" << endl;
	cout << "returnHead.type = " << returnHead.type << endl;
	cout << "returnHead.length = " << returnHead.length << endl;
	cout << "acceptfd = " << this->rsvHead.acceptfd << endl;

	//重新封装包
	char newReturnPacket[PACKET_SIZE] = { 0 };
	memcpy(newReturnPacket, &returnHead, sizeof(HEAD));							//添加协议头
	memcpy(newReturnPacket + sizeof(HEAD), returnBodyBuf, returnHead.length);	//添加协议体
	cout << "前置服务器新返回包封装完成 " << endl;

	//返回包发给客户端
	int writeRes = write(this->rsvHead.acceptfd, newReturnPacket, sizeof(HEAD)+ returnHead.length);
	if (0 < writeRes)
	{
		cout << "前置服务器新返回包发送成功" << endl;
		cout << " 拼合返回包 发回客户端 writeRes=" << writeRes << endl;
		--this->cilentMap->find(this->workfd)->second;
		this->ipc->ctlCount(1, 0);//发送包数+
	}
	else
	{
		cout << "前置服务器新返回包发送失败" << endl;
	}

	cout << "------------------------------------------------" << endl;
}

void CChildTask::setClient(map<int, int>* client)
{
	this->cilentMap = client;

}

void CChildTask::setHeart(map<int, int>* heart)
{
	this->heartBeatMap = heart;
}

uint32_t CChildTask::crc32(const char* data, size_t size)
{
	unsigned char i;
	uint32_t crc = POLY_INIT; /* 计算的初始crc值 */
	const uint32_t polynomial = POLYNOMIAL;

	while (size--)
	{
		crc ^= ((uint32_t)(*data++) << 24);
		for (i = 8; i > 0; --i)   /* 下面这段计算过程与计算一个字节crc一样 */
		{
			if (crc & 0x80000000)
				crc = (crc << 1) ^ polynomial;
			else
				crc = (crc << 1);
		}
	}
	crc = crc ^ POLY_END;
	return (crc);
}
