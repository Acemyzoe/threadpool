#include "CMsgQueue.h"
#include <functional>
#include <future>

class ThreadObject : public QueueObject
{
public:
    using QueueObject::QueueObject;
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

//线程池
class CThreadPool : public ThreadObject
{
public:
    using ThreadObject::ThreadObject;
    //启动默认线程池
    void Start(int nThreadNum = 1)
    {
        m_idlThrNum = nThreadNum < 1 ? 1 : nThreadNum;
        for (auto size = 0; size < nThreadNum; ++size)
        { //初始化线程数量
            m_pThreadPool.emplace_back(
                [this] { // 工作线程函数
                    while (!this->m_bStop)
                    {
                        std::function<void(void)> task;
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
                        task();
                        m_idlThrNum++;
                    }
                });
        }
    }

    //提交任务到线程池
    template <class F, class... Args>
    auto Commit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        using return_type = decltype(f(args...));

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_mu);

            //不允许向已停止的线程池提交作业
            if (m_bStop)
                throw std::runtime_error("向已停止的线程池提交作业");

            while (m_taskQueue.size() == m_nCapacity) //队列已满
            {
                m_condPush.wait(m_mu); //等待，将暂时的解锁
            }

            m_taskQueue.emplace([task]()
                                { (*task)(); });
        }
        m_condPop.notify_one();
        return res;
    }

    virtual size_t GetTaskNum()
    {
        return m_taskQueue.size();
    }

private:
    std::queue<std::function<void()>> m_taskQueue; //队列
};