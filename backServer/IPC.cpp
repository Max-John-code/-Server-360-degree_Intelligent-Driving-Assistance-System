#include "IPC.h"

IPC::IPC()
{
	//初始化
	this->shmArr[BLOCK_NUM] = { 0 };	//共享内存索引区 0—对应下标数据区空闲 1—对应下标数据区有数据
	this->sndIndex = -1;
	this->rsvIndex = -1;
	this->shmAddr = NULL;
	bzero(this->sndPacket, sizeof(this->sndPacket));
	bzero(this->rsvPacket, sizeof(this->rsvPacket));

	//创建共享内存
	this->shmID = shmget((key_t)1004, sizeof(shmArr) + sizeof(PACKET_SIZE) * BLOCK_NUM, IPC_CREAT | 0777);
	if (0 > shmID)
	{
		perror("shmget error");
	}
	else
	{
		cout << "共享内存创建完成" << endl;
	}

	//创建消息队列
	this->msgID = msgget((key_t)1005, IPC_CREAT | 0777);
	if (0 > msgID)
	{
		perror("msgget error");
	}
	else
	{
		cout << "消息队列创建完成" << endl;
	}

	//创建信号量
	this->semID = sem_create((key_t)1006, BLOCK_NUM);	//共享内存数据区个数=信号量个数
	cout << "信号量创建完成" << endl;

	//创建日志共享内存
	this->shmid2 = shmget((key_t)2001, sizeof(workNum), IPC_CREAT | 0777);
	if (this->shmid2 < 0)
	{
		perror("log shmget error");
		exit(0);
	}
	cout << "日志专用共享内存创建成功 " << shmid2 << endl;
	this->semid2 = sem_create((key_t)2002, BLOCK_NUM);
	cout << "日志信号量初始化完毕 " << semid2 << endl;
}

IPC::~IPC()
{
	delete this->shmAddr;
	delete this->shmArr;
	delete this->sndPacket;
}

int IPC::sem_create(key_t key, int num_sems)
{
	int res = semget(key, num_sems, IPC_CREAT | 0777);
	if (res < 0)
	{
		perror("semget error!!!");
	}
	return res;

}

void IPC::sem_setVal(int sem_id, int sem_num, int value)
{
	union semun arg;
	arg.val = value;
	//信号量初始化
	//semID：哪个信号量；semsndIndex：下标；value：具体数值
	int res = semctl(sem_id, sem_num, SETVAL, arg);
	if (res < 0)
	{
		perror("semctl error!!!");
	}

}

void IPC::sem_p(int sem_id, int sem_index)
{
	struct sembuf buf = { sem_index ,-1,SEM_UNDO };
	//1--一一口气完成
	int res = semop(sem_id, &buf, 1);
	if (res < 0)
	{
		perror("semop P error!!!");
	}

}

void IPC::sem_v(int sem_id, int sem_index)
{
	struct sembuf buf = { sem_index ,1,SEM_UNDO };
	//1--一一口气完成
	int res = semop(sem_id, &buf, 1);
	if (res < 0)
	{
		perror("semop V error!!!");
	}

}

void IPC::setShm(char resbuf[])
{
	cout << "------------------------------------------------" << endl;

	//接收业务包
	bzero(this->sndPacket, sizeof(this->sndPacket));
	memcpy(this->sndPacket, resbuf, sizeof(this->sndPacket));

	//共享内存操作
	this->shmAddr = shmat(this->shmID, NULL, 0);			//连接共享内存

	memcpy(this->shmArr, this->shmAddr, sizeof(this->shmArr));	//获取当前共享内存索引区信息
	while (1)
	{
		for (int i = 0; i < BLOCK_NUM; i++)
		{
			if (0 == this->shmArr[i])
			{
				this->sndIndex = i;	//若索引区对应数据域空闲，记录当前索引区下标
				break;
			}
		}
		if (this->sndIndex >= 0 && this->sndIndex < BLOCK_NUM)
		{
			break;
		}
	}
	
	cout << "共享内存当前操作索引区下标 sndIndex = " << sndIndex << endl;

	//对应信号量加锁
	sem_p(this->semID,this->sndIndex);

	//数据写入共享内存
	memcpy(shmAddr + sizeof(shmArr) + PACKET_SIZE * sndIndex, &this->sndPacket, PACKET_SIZE);	//操作数据区：首地址 + 索引区总长度 + 数据区*下标
	shmArr[sndIndex] = 1;
	memcpy(shmAddr + sizeof(int) * sndIndex, &shmArr[sndIndex], sizeof(int));					//操作索引区：首地址 + 一个索引大小*下标
	cout << "前置服务器新包丢进共享内存 " << endl;

	
	//测试：直接读出
	/*char data[PACKET_SIZE] = {0};
	memcpy(&data, this->shmAddr + sizeof(shmArr) + PACKET_SIZE *this->sndIndex, PACKET_SIZE);

	REBUILD_HEAD head = { 0 };
	memcpy(&head, data, sizeof(REBUILD_HEAD));
	cout << "-----------------------------------------------" << endl;
	cout << "测试读取业务包头" << endl;
	cout << "head.length = " << head.length << endl;
	cout << "head.type = " << head.type << endl;
	cout << "head.acceptfd = " << head.acceptfd << endl;
	USER body = { 0 };
	memcpy(&body, data + sizeof(REBUILD_HEAD), head.length);
	cout << "测试读取业务包体" << endl;
	cout << "body.id = " << body.name << endl;
	cout << "body.password = " << body.pwd << endl;
	cout << "-----------------------------------------------" << endl;*/

	//共享内存断开链接
	shmdt(this->shmAddr);

	//对应信号量解锁
	sem_v(this->semID,this->sndIndex);

	//发送消息至消息队列，mtype=2：提示前端服务器读取数据
	MSGBUF buf = { 0 };
	buf.mtype = 2;
	sprintf(buf.mtext, "%d", sndIndex);			//消息初始化

	if (-1 == msgsnd(this->msgID, &buf, sizeof(buf.mtext), 0))	//消息发送
	{
		perror("msgsnd error");
	}
	cout << "消息发送成功" << endl;

	//数据初始化
	bzero(&shmArr[sndIndex], sizeof(int));
	this->sndIndex = -1;

	cout << "------------------------------------------------" << endl;
}

void IPC::getShm()
{
	cout << "------------------------------------------------" << endl;

	//接收前置服务器发来的消息 mtype=1
	//this->buf = { 0 };
	bzero(&(this->buf), sizeof(this->buf));
	if (-1 == msgrcv(this->msgID, &buf, sizeof(buf.mtext), 1, 0))	//接收消息
	{
		perror("msgrcv error");
	}
	this->rsvIndex = atoi(buf.mtext);
	cout << "接收消息成功" << endl;
	cout << "收到共享内存索引区下标 rsvIndex = " << this->rsvIndex << endl;

	//共享内存操作
	shmAddr = shmat(shmID, NULL, 0);	//连接共享内存

	memcpy(&this->shmArr[rsvIndex], this->shmAddr + sizeof(int) * this->rsvIndex, sizeof(int));				//读索引区：首地址 + 一个索引*下标
	if (1 == this->shmArr[rsvIndex])					//若索引区对应数据区内有数据
	{
		sem_p(this->semID,this->rsvIndex);		//对应信号量加锁

			//从共享内存中读取数据
			//读数据区：首地址 + 整个索引区长度 + 数据块*下标
		bzero(&rsvPacket, PACKET_SIZE);		//数据初始化
		memcpy(&this->rsvPacket, this->shmAddr + sizeof(shmArr) + PACKET_SIZE * this->rsvIndex, PACKET_SIZE);

		//测试：获取的返回包
		/*REBUILD_HEAD head = {0};
		memcpy(&head, this->rsvPacket, sizeof(REBUILD_HEAD));
		cout << "-----------------------------------------------" << endl;
		cout << "测试读取返回包头" << endl;
		cout << "head.length = " << head.length << endl;
		cout << "head.type = " << head.type << endl;
		cout << "head.acceptfd = " << head.acceptfd << endl;
		LOGINBACK body = { 0 };
		memcpy(&body, this->rsvPacket + sizeof(REBUILD_HEAD), head.length);
		cout << "测试读取返回包体" << endl;
		cout << "body.result = " << body.result << endl;
		cout << "body.page = " << body.page << endl;
		cout << "-----------------------------------------------" << endl; */

		//清空读出的数据区数据
		//修改索引区为空闲—0
		memset(this->shmAddr + sizeof(this->shmArr) + PACKET_SIZE * this->rsvIndex, 0, PACKET_SIZE);
		memset(this->shmAddr + sizeof(int) * this->rsvIndex, 0, sizeof(int));

		//共享内存断开链接
		shmdt(this->shmAddr);

		sem_v(this->semID,this->rsvIndex);		//对应信号量解锁

		//数据初始化
		bzero(&buf, sizeof(MSGBUF));
		bzero(&this->shmArr[rsvIndex], sizeof(int));
		this->rsvIndex = -1;
	}

	cout << "------------------------------------------------" << endl;
}

char* IPC::getRsvPacket()
{
	return this->rsvPacket;
}

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

void IPC::ctlCount(int index, int mode)
{
	void* shmaddr2 = NULL;

	//加锁 p操作
	sem_p(semid2, index);
	shmaddr2 = shmat(shmid2, NULL, 0);
	memcpy(workNum, shmaddr2, sizeof(workNum)); //拷贝数据
	//进程内本地数组操作 
	if (mode == 0)
		++workNum[index];
	else if (mode == 1)
		--workNum[index];
	//操作索引区 只能拷贝数组中被修改的哪一个元素
	memcpy((void*)(shmaddr2 + index * sizeof(int)), &workNum[index], sizeof(int));
	shmdt(shmaddr2);
	sem_v(semid2, index);
	bzero(&workNum[index], sizeof(int));
}