#ifndef _THREADPOOL_H
#define _THREADPOOL_H
#include <vector>
#include <queue>
#include <thread>
#include <iostream>
#include <condition_variable>
using namespace std;

const int MAX_THREADS = 10; //最大线程数目

template <typename T>
class threadPool
{
public:
    threadPool(int number = 1);
    ~threadPool();
    bool enqueue(T *task);
    //工作线程需要运行的函数,不断的从任务队列中取出并执行
    static void *worker(void *arg);
    void run();

private:
    //工作线程
    vector<thread> workThread;
    //任务队列
    queue<T *> taskQueue;
    mutex mt;
    condition_variable condition;
    bool stop;
};

template <typename T>
threadPool<T>::threadPool(int number) : stop(false)
{
    if (number <= 0 || number > MAX_THREADS)
        throw exception();
    for (int i = 0; i < number; i++)
    {
        cout << "create thread：" << i << endl;
        workThread.emplace_back(worker, this);
    }
}

template <typename T>
inline threadPool<T>::~threadPool()
{
    {
        unique_lock<mutex> unique(mt);
        stop = true;
    }
    condition.notify_all();
    for (auto &wt : workThread)
        wt.join();
}

template <typename T>
bool threadPool<T>::enqueue(T *task)
{
    //往任务队列添加任务的时候，要加锁，因为这是线程池，肯定有很多线程
    unique_lock<mutex> unique(mt);
    taskQueue.push(task);
    unique.unlock();
    //任务添加完之后，通知阻塞线程过来消费任务，有点像生产消费者模型
    condition.notify_one();
    return true;
}

template <typename T>
void *threadPool<T>::worker(void *arg)
{
    threadPool *pool = (threadPool *)arg;
    pool->run();
    return pool;
}

template <typename T>
void threadPool<T>::run()
{
    while (!stop)
    {
        unique_lock<mutex> unique(this->mt);
        //如果任务队列为空，就停下来等待唤醒，等待另一个线程发来的唤醒请求
        while (this->taskQueue.empty())
            this->condition.wait(unique);
        T *task = this->taskQueue.front();
        this->taskQueue.pop();
        if (task)
            task->process();
    }
}
#endif