/*
 * Copyright (C) 2011-2022 sgcc Inc.
 * All right reserved.
 * 文件名称：ThreadPool.h
 * 摘    要：线程池库（固定线程数量）
 * 作者：GuoJun  完成日期：2022/04/01
 */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>             // std::thread 线程相关
#include <mutex>              // std::mutex, std::unique_lock 互斥量
#include <condition_variable> // 条件量，用于线程间通信，唤醒阻塞线程
#include <future>             // std::future 获取线程数据
#include <functional>         // std::function 函数对象
#include <stdexcept>          // std::runtime_error   标准异常

class ThreadPool // 线程池类
{
public:
    ThreadPool(size_t); // 构造函数，初始化线程池
    //一个enqueue模板函数 返回std::future<type>, type利用了运行时检测推断出类型
    //可以提交变参函数或拉姆达表达式的匿名函数执行,可以获取执行返回值
    template <class F, class... Args>                              // 模板函数，接受任意参数
    auto enqueue(F &&f, Args &&...args)                            // 将任务添加到任务队列中
        -> std::future<typename std::result_of<F(Args...)>::type>; // 返回任务执行结果
    ~ThreadPool();                                                 // 析构函数，终止线程池

private:
    std::vector<std::thread> workers;        // 工作线程
    std::queue<std::function<void()>> tasks; // 任务队列

    // synchronization
    std::mutex queue_mutex;            // 互斥量，用于线程间同步
    std::condition_variable condition; // 条件量，用于线程间同步
    bool stop;                         // 线程池是否终止
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
    : stop(false) // 初始化线程池
{
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back(
            [this]
            {
                for (;;)
                {
                    std::function<void()> task; // 任务函数对象
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex); // 互斥量，用于线程间同步
                        this->condition.wait(lock,
                                             [this]
                                             { return this->stop || !this->tasks.empty(); }); // 等待条件量，等待任务队列不为空（condition是返回false才会wait）
                        if (this->stop && this->tasks.empty())                                // 如果线程池终止，且任务队列为空
                            return;
                        task = std::move(this->tasks.front()); // 取出任务队列中的第一个任务
                        this->tasks.pop();                     // 删除任务队列中的第一个任务
                    }
                    task(); // 执行任务
                }
            });
}

// add new work item to the pool
template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args)               // 接受任意参数
    -> std::future<typename std::result_of<F(Args...)>::type> // 返回任务执行结果
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future(); // 获取任务执行结果
    {                                                  // 加锁入队
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task]()
                      { (*task)(); }); // 将任务添加到任务队列中
    }
    condition.notify_one(); // 通知工作线程
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();             // 唤醒所有线程
    for (std::thread &worker : workers) // 等待所有线程结束
        worker.join();
}

#endif
