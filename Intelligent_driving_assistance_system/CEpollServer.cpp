#include "CEpollServer.h"

CEpollServer::CEpollServer(unsigned short port)
{
	this->tcp = new CTcpServer(port, SOCK_STREAM);
	this->tcp->work();
	this->socketfd = this->tcp->getSocketfd();
	this->acceptfd = 0;
	this->epollWaitNum = 0;
	this->res = 0;
	this->allFdNum = 0;
	//创建epoll epollfd代表epoll这个对象 (红黑树根结点)
	//同时监听fd的数量 必须和就绪列表数量保持一致
	this->epollfd = epoll_create(EPOLL_SIZE);

	//线程池对象
	this->pool = new CThreadPool();
	//共享内存初始化
	this->ipc = new IPC();

	bzero(this->packet, sizeof(this->packet));
	bzero(&(this->head), sizeof(this->head));
	bzero(&(this->epollevent), sizeof(epollevent));
	bzero(this->packet, sizeof(this->packet));
	bzero(this->epolleventArray, sizeof(this->epolleventArray));

	//创建epoll接收后置服务器返回包线程
	pthread_t pid;
	pthread_create(&pid, NULL, epollThreadFun, this);

	//打印实时日志
	pthread_t pid2;
	pthread_create(&pid2, NULL, printfLogsThread, this);
	//创建心跳检测业务线程
	pthread_t heartPid;
	pthread_create(&heartPid, NULL, heartBeatThreadFun, this);

	//网络连接的用户,但是还没有登录
	pthread_t pid3;
	pthread_create(&pid3, NULL, socketOnline, this);

	EpollControl(EPOLL_CTL_ADD, this->socketfd, 1);///前置条件：将服务器socketfd加入epoll事件数组中
}

CEpollServer::~CEpollServer()
{
	delete this->tcp;
	delete this->epolleventArray;
	delete this->pool;
	delete this->packet;
}

void CEpollServer::setSocketfd(int fd)
{
	this->socketfd = fd;
}

void CEpollServer::EpollControl(int op, int fd, int type)
{
	bzero(&(this->epollevent), sizeof(this->epollevent));

	this->epollevent.data.fd = fd;

	if (type == 1)
	{
		//数据到来以后事情驱动方式默认为LT
		this->epollevent.events = EPOLLIN;
	}
	else if (type == 2)
	{
		//数据到来以后事情驱动方式为ET
		this->epollevent.events = EPOLLIN | EPOLLET;
	}

	//将配置好的epollevent事件结构体 保存到epoll事件链表中
	epoll_ctl(this->epollfd, op, fd, &(this->epollevent));

	/*
	*
	* epoll 事件驱动型IO
	* 提供内核识别网络数据机制:如果数据到来(掉线,发请求,链接),识别是哪个fd,
	* 从fd的红黑树中取出fd进行读数据操作,,去提醒工作进程,进行读数据操作
	*
	只有EPOLLIN，默认epolll模式采用LT模式
	用上了EPOLLET，修改epolll模式采用ET模式

	LT--水平触发
	ET--边缘(沿)触发

	客户端100字节数据-》事件提醒-》服务器读数据-》先读头再读体
	服务器第一次fd触发还没有读取完网络通道里的数据
	这个时候同样的客户端同样的fd有新的数据到来

	LT模式:只有有数据来，无论网络通道里数据是否操作完，都会再次提醒,直到全部读完为止不再提醒
	造成资源浪费

	ET模式:数据到来以后就提醒一次,除非有新的数据到来,才会再次提醒
	减少资源浪费

	*/
}

void CEpollServer::EpollWork()
{
	//因为服务器是24h长时间开机
	while (1)
	{
		//-1是一种状态永远等待
		//epollWaitNum 代表从epoll事件触发以后 取出多少个fd
		this->epollWaitNum = epoll_wait(this->epollfd, this->epolleventArray, EPOLL_SIZE, -1);
		//eopll等待事件发生
		//执行成功返回执行的事件数目，失败返回-1，超时返回0
		if (this->epollWaitNum < 0)
		{
			perror("epoll_wait error ");
			return;
		}
		else
		{
			//有事件发生： 1.有客户端连接                                ---添加到epoll事件列表
			//             2.客户端发送数据                              ---读取数据
			//             3.客户端下线(正常退出、终端、kill、异常退出） ---从epoll事件链表删除
			for (int i = 0; i < this->epollWaitNum; i++)
			{
				if (this->epolleventArray[i].data.fd == this->socketfd)
				{
					cout << "有客户端来连接服务器...." << endl;
					//acceptfd 代表已经连接成功的客户端
					//阻塞式函数：等待用户连接
					this->acceptfd = accept(this->socketfd, NULL, NULL);
					cout << "客户端已经上线  acceptfd =" << this->acceptfd << endl;
					//完成连接的客户端放入epoll
					EpollControl(EPOLL_CTL_ADD, this->acceptfd, 2);
					cout << "向map添加fd" << acceptfd << endl;
					this->cilentMap.insert(make_pair(acceptfd, -1)); //没有0个业务处理中
					this->onlineClients.insert(make_pair(acceptfd, 0)); //用户上线
					cout << "client添加用户 fd=" << acceptfd << endl;
					++allFdNum;

				}
				else if (this->epolleventArray[i].events & EPOLLIN)//数组中有事件 且 事件是EPOLLIN：客户端请求业务处理
				{
					cout << "有事件发生，但不是socket，是客户端有操作" << endl;

					//读数据判断是否有数据：有：客户端业务；无：客户端下线
					bzero(&(this->head), sizeof(this->head));
					bzero(this->packet, sizeof(this->packet));
					this->res = read(epolleventArray[i].data.fd, &(this->head), sizeof(this->head));
					if (res > 0)
					{
						//正常的业务处理在这
						cout << "客户端: " << this->epolleventArray[i].data.fd << "发来数据" << endl;
						if (this->head.type == HEART_BEAT)
						{
							//创建子任务
							//心跳处理
							 CBaseTask*  task = new CHeartBeatTask(this->packet, epolleventArray[i].data.fd, &this->heartBeatMap);
							 //线程池添加任务
							 this->pool->pushTask(task);
						}
						else
						{
							memcpy(this->packet, &(this->head), sizeof(this->head));
							this->res = read(epolleventArray[i].data.fd, this->packet+sizeof(this->head), head.length);
							//创建子任务
							CBaseTask* task = new CChildTask(this->packet, SEND_TO_BACK,&this->cilentMap,&this->heartBeatMap);

							task->setWorkfd(epolleventArray[i].data.fd);	//设置客户端acceptfd
							task->setIPC(this->ipc);						//设置共享内存
							
							//线程池添加任务
							this->pool->pushTask(task);
						}
					}
					else
					{
						cout << "客户端下线了" << endl;
						EpollControl(EPOLL_CTL_DEL, this->epolleventArray[i].data.fd, 2);
						//关闭下线的客户端
						close(this->epolleventArray[i].data.fd);
						--allFdNum;
						this->ipc->ctlCount(5, 0);
						cout << "从map删除fd" << epolleventArray[i].data.fd << endl;
						this->cilentMap.erase(epolleventArray[i].data.fd); //0个业务处理中 
						this->heartBeatMap.erase(epolleventArray[i].data.fd);
						this->onlineClients.erase(epolleventArray[i].data.fd);
					}
				}

			}
		}
	}
}

void CEpollServer::updateOnlineClient(int fd)
{
	//检索用户Id账号是否在
	/*map<int, int>::iterator map_iter = this->onlineClients.find(fd);
	map<int, int>::iterator map_iter2 = this->onlineClients_heartBeat.find(fd);
	if (map_iter != this->onlineClients.end())
	{
		//如果用户账号在线
		//更新用户操作时间
		map_iter2->second++;		//只更新heartBeatMap

		cout << "用户数据更新" << endl;
		cout << "fd = " << map_iter2->first << endl;
		cout << "time = " << map_iter2->second << endl;
	}
	else
	{
		//如果用户Id账号不在线
		//map新增数据
		this->onlineClients[fd] = 0;
		this->onlineClients_heartBeat[fd] = 1;
		cout << "新用户上线" << endl;
	}
	*/
	cout << "----------------------------在线用户数据更新成功----------------------------" << endl;
}

void CEpollServer::deleteClient(int fd)
{
	/*
	map<int, int>::iterator map_iter = this->onlineClients.begin();
	map<int, int>::iterator map_iter2 = this->onlineClients_heartBeat.begin();
	for (; map_iter != this->onlineClients.end() && map_iter2 != this->onlineClients_heartBeat.end(); map_iter++, map_iter2++)
	{
		if (map_iter->first == fd)
		{
			this->onlineClients.erase(map_iter);
		}

		if (map_iter2->first == fd)
		{
			this->onlineClients_heartBeat.erase(map_iter2);
		}
	}
	*/
	cout << "在线用户数据删除" << endl;
}

void* CEpollServer::epollThreadFun(void* arg)
{
	//线程执行函数中,获取运行线程本身自己的id号
	pthread_t threadid = pthread_self();

	//确保主线程与当前执行的线程的逻辑完全分离,当前线程执行结束,id会自动释放:让线程号足够使用
	//分离的目的:为了声明这个线程不会阻塞主线程,pthread_detach不会终止线程的运行
	pthread_detach(threadid);

	CEpollServer* epoll = (CEpollServer*)arg;

	while (true)
	{
		cout << "epoll返回业务执行中.............." << endl;

		//等待后置返回消息和返回包
		epoll->ipc->getShm();

		//创建从后置接收返回包任务
		CBaseTask* task = new CChildTask("", GET_FROM_BACK, &epoll->cilentMap, &epoll->heartBeatMap);

		//任务设置共享内存
		task->setIPC(epoll->ipc);

		//线程池添加任务
		epoll->pool->pushTask(task);
	}
}

void* CEpollServer::printfLogsThread(void* arg)
{
	CEpollServer* epoll = (CEpollServer*)arg;
	while (1)
	{
		sleep(10);
		epoll->ipc->printfLog(epoll->allFdNum);
	}
	return nullptr;
}

void* CEpollServer::socketOnline(void* arg)
{
	CEpollServer* epoll = (CEpollServer*)arg;
	while (1)
	{
		list<int> temp;
		list<int> earseUser;
		map<int, int>::iterator it;
		cout << endl << "========================= 网络连接用户检测 =============================" << endl;
		for (map<int, int>::iterator iter = epoll->onlineClients.begin(); iter != epoll->onlineClients.end(); iter++)
		{
			++(iter->second);
			cout << "网络连接客户端 fd =" << iter->first << " 计次" << iter->second << endl;
			if (iter->second == 6)
			{
				cout << "长时间无反应，强制下线 fd " << iter->first << endl;
				epoll->EpollControl(iter->first, EPOLL_CTL_DEL, 2);
				cout << "客户端 " << iter->first << " 已掉线" << endl;
				close(iter->first);//关闭客户端fd
				cout << "从map删除fd" << iter->first << endl;
				cout << "被删除的client的业务数 num =" << epoll->cilentMap[iter->first] << endl;
				cout << "被删除的heart的业务数 统计数 =" << epoll->heartBeatMap[iter->first] << endl;
				temp.push_back(iter->first);
				cout << "删除完毕" << endl;
				--epoll->allFdNum;
			}
			it = epoll->heartBeatMap.find(iter->first);
			if (it != epoll->heartBeatMap.end())
			{
				earseUser.push_back(iter->first);
			}
			
		}

		for (list<int>::iterator iter = temp.begin(); iter != temp.end(); iter++)
		{
			epoll->cilentMap.erase(*iter); //0个业务处理中 
			epoll->heartBeatMap.erase(*iter);
			cout << "正在删除 fd =" << *iter << endl;
		}

		for (list<int>::iterator iter = earseUser.begin(); iter != earseUser.end(); iter++)
		{
			epoll->onlineClients.erase(*iter); //删除用户
		}

		cout << "==================================================================================" << endl;
		sleep(10);
	}

	return nullptr;
}

void* CEpollServer::heartBeatThreadFun(void* arg)
{
	CEpollServer* epoll = (CEpollServer*)arg;

	while (1)
	{
		list<int> temp;
		cout << endl << "@@@@@@@@@@@@@@@@@@@@@@@@ 心跳检测 @@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
		for (map<int, int>::iterator iter = epoll->heartBeatMap.begin(); iter != epoll->heartBeatMap.end(); iter++)
		{
			++(iter->second);
			cout << "客户端 fd =" << iter->first << " 计次" << iter->second << endl;
			if (iter->second == 3)
			{
				cout << "长时间无反应，强制下线 fd " << iter->first << endl;
				epoll->EpollControl(iter->first, EPOLL_CTL_DEL,2);
				cout << "客户端 " << iter->first << " 已掉线" << endl;
				close(iter->first);//关闭客户端fd
				cout << "从map删除fd" << iter->first << endl;
				cout << "被删除的client的业务数 num =" << epoll->cilentMap[iter->first] << endl;
				cout << "被删除的heart的业务数 统计数 =" << epoll->heartBeatMap[iter->first] << endl;
				temp.push_back(iter->first);
				cout << "删除完毕" << endl;
				--epoll->allFdNum;
			}
		}

		for (list<int>::iterator iter = temp.begin(); iter != temp.end(); iter++)
		{
			epoll->cilentMap.erase(*iter); //0个业务处理中 
			epoll->heartBeatMap.erase(*iter);
			cout << "正在删除 fd =" << *iter << endl;
		}

		cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
		sleep(10);
	}
	return nullptr;
}

