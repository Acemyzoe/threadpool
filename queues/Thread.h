#pragma once
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#define MAX_QUEUE_CAPACITY 100

// Queue class
class QueueObject
{
public:
    QueueObject() : m_bStop(false), m_nCapacity(MAX_QUEUE_CAPACITY)
    {
    }

    virtual ~QueueObject()
    {
    }

    void Stop()
    {
        m_bStop.store(true);
        m_condPop.notify_all(); // 唤醒所有线程执行
    }

    //设置最大容量
    void SetMaxCapacity(int nMax)
    {
        m_nCapacity = nMax;
    }

    //获取队列任务数量
    virtual size_t GetTaskNum() = 0;

    bool IsStop()
    {
        return m_bStop;
    }

protected:
    int m_nCapacity = 0;                    //队列最大容量
    std::condition_variable_any m_condPush; //写入条件量
    std::condition_variable_any m_condPop;  //读取条件量
    std::mutex m_mu;                        //互斥锁

    // 是否关闭提交
    std::atomic<bool> m_bStop;
};

// thread safe queue
class ThreadObject : public QueueObject
{
public:
    using QueueObject::QueueObject;
    ThreadObject() {}
    ~ThreadObject()
    {
        Stop();
        for (std::thread &thread : m_pThreadPool)
        {
            if (thread.joinable())
                thread.join(); // 等待任务结束， 前提：线程一定会执行完
        }
    }

    int ThreadCount()
    {
        return m_pThreadPool.size();
    }

    //空闲线程数量
    int IdlCount() { return m_idlThrNum; }

protected:
    //空闲线程数量
    std::atomic<int> m_idlThrNum;
    std::vector<std::thread> m_pThreadPool;
};