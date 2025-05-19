#include "CThreadPool.h"

CThreadPool::CThreadPool(const int num)
{
    this->threadMinNum = num;
    //互斥量初始化
    pthread_mutex_init(&(this->mutex), NULL);
    //条件变量初始化
    pthread_cond_init(&(this->cond), NULL);

    for (int i = 0; i < this->threadMinNum; ++i)
    {
        pthread_t id;
        //静态成员想使用非静态的成员需要使用传参
        pthread_create(&id, NULL, pthread_function, this);
        //创建好的线程添加到空闲链表
        this->idleList.push_back(id);
    }
}

CThreadPool::~CThreadPool()
{
}
//添加任务
void CThreadPool::pushTask(CBaseTask* task)
{
    //只有主线程调用这个数据,epoll使用,所以不用加锁
    this->taskQueue.push(task);
    //判断空闲的队列是否为空,为空需要添加线程
    //条件是需要空闲列表为空+忙碌列表的大小小于最大的线程数,并且大于最小值
    if ((this->busyList.size() + this->idleList.size()) < MAX_THREAD_NUM
        && (this->busyList.size() + this->idleList.size()) > MIN_THREAD_NUM)
    {
        //创建线程,添加到空闲列表
        pthread_t id;
        //静态成员想使用非静态的成员需要使用传参
        pthread_create(&id, NULL, pthread_function, this);
        //创建好的线程添加到空闲链表
        this->idleList.push_back(id);
    }

    this->wakeup();

}



//一个问题:
//如果已经创建到了最大的线程数,但是,还有一个任务在等待分配线程,这个时候怎么办
//这个函数是外部调用,只走一次,一次不过.后面怎么让这个任务执行?
CBaseTask* CThreadPool::popTask()
{
    //因为本身的锁还未解开,所以不用加锁
    CBaseTask* task = this->taskQueue.front();
    this->taskQueue.pop();
    return task;
}
//从忙碌链表删,添加到空闲链表
void CThreadPool::moveToIdle(pthread_t id)
{
    list<pthread_t>::iterator iter = find(this->busyList.begin(), this->busyList.end(), id);
    if (iter != this->busyList.end())
    {
        //找到了
        //忙碌链表删除
        this->busyList.erase(iter);
        //空闲链表添加
        this->idleList.push_back(*iter);
    }
}
//从空闲链表删除,添加到忙碌链表
void CThreadPool::moveToBusy(pthread_t id)
{
    list<pthread_t>::iterator iter = find(this->idleList.begin(), this->idleList.end(), id);
    if (iter != this->idleList.end())
    {
        //找到了
        //空闲链表删除
        this->idleList.erase(iter);
        //忙碌链表添加
        this->busyList.push_back(*iter);
    }
}
//判断任务队列是否为空
bool CThreadPool::queueIsEmpty()
{
    return this->taskQueue.empty();
}

void CThreadPool::lock()
{
    pthread_mutex_lock(&(this->mutex));
}

void CThreadPool::unlock()
{
    pthread_mutex_unlock(&(this->mutex));
}
//阻塞线程
void CThreadPool::wait()
{
    //先加锁才能使用
    pthread_cond_wait(&(this->cond), &(this->mutex));
}
//唤醒线程
void CThreadPool::wakeup()
{
    pthread_cond_signal(&(this->cond));
}

void* CThreadPool::pthread_function(void* arg)
{
    //线程执行函数中,获取运行线程本身自己的id号
    pthread_t threadid = pthread_self();

    //确保主线程与当前执行的线程的逻辑完全分离,当前线程执行结束,id会自动释放:让线程号足够使用
    //分离的目的:为了声明这个线程不会阻塞主线程,pthread_detach不会终止线程的运行
    pthread_detach(threadid);

    CThreadPool* pool = (CThreadPool*)arg;
    while (1)
    {
        //上锁--其余9个线程无法lock
        pool->lock();
        while (pool->queueIsEmpty())
        {
            //阻塞线程 一个线程因为阻塞过后,后面线程会被锁卡柱
            pool->wait();
        }
        cout << "线程工作前 任务数:" << pool->taskQueue.size() << endl;
        cout << "线程工作前 忙碌线程数:" << pool->busyList.size() << endl;
        cout << "线程工作前 空闲线程数: " << pool->idleList.size() << endl;
        cout << "----------------------------------------------------" << endl;

        //从空闲链表删除,添加到忙碌链表
        pool->moveToBusy(threadid);
        //从任务队列取任务
        CBaseTask* task = pool->popTask();
        //解锁
        pool->unlock();

        //任务开始工作
        task->working();

        //加锁---空闲链表也是公共数据
        pool->lock();
        //从忙碌链表删,添加到空闲链表
        pool->moveToIdle(threadid);

        //查看任务队列中是否还有任务,没有并且忙碌列表+空闲列表的大小大于最小的线程数开始删除线程
        if (pool->taskQueue.empty() && (pool->busyList.size() + pool->idleList.size()) > MIN_THREAD_NUM)
        {
            while ((pool->busyList.size() + pool->idleList.size()) > MIN_THREAD_NUM)
            {
                //空闲列表尾删线程
                pool->idleList.pop_back();
            }
        }

        //解锁
        pool->unlock();

        cout << "线程工作后 任务数:" << pool->taskQueue.size() << endl;
        cout << "线程工作后 忙碌线程数:" << pool->busyList.size() << endl;
        cout << "线程工作后 空闲线程数: " << pool->idleList.size() << endl;
        cout << "----------------------------------------------------" << endl;
    }


    return nullptr;
}
