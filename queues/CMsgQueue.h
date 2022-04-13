#pragma once
#include "Thread.h"

// msg queue
template <typename T, typename... ARGS>
class CMsgQueue : public QueueObject
{
public:
    using QueueObject::QueueObject;
    void Push(T val, const ARGS... args)
    {
        while (m_dataQueue.size() == m_nCapacity) // 队列已满
        {
            m_condPush.wait(m_mu); // 等待，将暂时的解锁
        }

        m_dataQueue.emplace(std::make_tuple(val, args...));

        m_condPop.notify_one(); // 唤醒一个线程执行
    }

    // 批量获取参数值
    bool Pop(std::tuple<T, ARGS...> &value, int waitTime = -1)
    {
        std::unique_lock<std::mutex> lock(m_mu);
        if (waitTime < 0)
        {
            this->m_condPop.wait(lock,
                                 [this]
                                 {
                                     return !this->m_dataQueue.empty();
                                 }); // wait 直到有 task
        }
        else
        {
            auto status = m_condPop.wait_for(lock, std::chrono::seconds(waitTime), [this]
                                             { return !this->m_dataQueue.empty(); });
            if (!status)
            {
                return false;
            }
        }

        value = std::move(this->m_dataQueue.front()); // 取一个 task
        this->m_dataQueue.pop();

        //通知写线程
        m_condPush.notify_one();

        return true;
    }

    bool Pop(T &value, ARGS &...args, int waitTime = -1)
    {
        std::tuple<T, ARGS...> tupVal;
        if (Pop(tupVal, waitTime))
        {
            FetchParam<0>(tupVal, value, args...);
            return true;
        }
        return false;
    }

    template <int NUM, typename P, typename... PARMS>
    void FetchParam(std::tuple<T, ARGS...> &tupVal, P &p, PARMS &...params)
    {
        p = std::get<NUM>(tupVal);
        FetchParam<NUM + 1>(tupVal, params...);
    }

    template <int NUM, typename P>
    void FetchParam(std::tuple<T, ARGS...> &tupVal, P &p)
    {
        p = std::get<NUM>(tupVal);
    }

    //获取队列任务数量
    virtual size_t GetTaskNum()
    {
        return m_dataQueue.size();
    }

private:
    std::queue<std::tuple<T, ARGS...>> m_dataQueue;
};