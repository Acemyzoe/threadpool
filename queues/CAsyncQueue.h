#pragma once
#include "Thread.h"
#include <functional>
#include <future>

//  T 数据类型
//  RET 结果返回类型
template <typename T, typename RET>
struct NodeData
{
    T data;
    std::promise<RET> res;
};

//  异步队列
//  用户将数据压入队列，并自定义执行函数，对数据进行异步处理
template <typename T, typename RET>
class CAsyncQueue : public ThreadObject
{
public:
    void Start(unsigned short nThreadNum, std::function<RET(T)> f)
    {
        m_idlThrNum = nThreadNum < 1 ? 1 : nThreadNum;
        for (auto size = 0; size < nThreadNum; ++size)
        { //初始化线程数量
            m_pThreadPool.emplace_back(
                [this, f] { // 工作线程函数
                    while (!this->m_bStop)
                    {
                        NodeData<T, RET> task;
                        { // 获取一个待执行的 task
                            // unique_lock 相比 lock_guard 的好处是：可以随时 unlock() 和 lock()
                            std::unique_lock<std::mutex> lock(m_mu);

                            this->m_condPop.wait(lock,
                                                 [this]
                                                 {
                                                     return this->m_bStop.load() || !this->m_taskQueue.empty();
                                                 }); // wait 直到有 task
                            if (this->m_bStop && this->m_taskQueue.empty())
                                return;
                            task = std::move(this->m_taskQueue.front()); // 取一个 task
                            this->m_taskQueue.pop();
                        }
                        //通知写线程
                        m_condPush.notify_one();
                        m_idlThrNum--;
                        task.res.set_value(f(task.data));
                        m_idlThrNum++;
                    }
                });
        }
    }

    //向队列中压入任务
    std::future<RET> Push(T val)
    {
        std::promise<RET> prs;
        auto ret = prs.get_future();

        {
            std::unique_lock<std::mutex> lock(m_mu);

            //不允许向已停止的线程池提交作业
            if (m_bStop)
                throw std::runtime_error("向已停止的线程工厂提交作业");

            while (m_taskQueue.size() == m_nCapacity) //队列已满
            {
                m_condPush.wait(m_mu); //等待，将暂时的解锁
            }

            m_taskQueue.emplace(NodeData<T, RET>{val, std::move(prs)});
        }

        m_condPop.notify_one(); // 唤醒一个线程执行
        return ret;
    }

    virtual size_t GetTaskNum()
    {
        return m_taskQueue.size();
    }

private:
    std::queue<NodeData<T, RET>> m_taskQueue; //队列
};