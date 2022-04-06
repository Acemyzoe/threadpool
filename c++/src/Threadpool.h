/*
 * Copyright (C) 2011-2022 sgcc Inc.
 * All right reserved.
 * 文件名称：ThreadPool.h
 * 摘    要：动态线程池库（动态增加线程数量）
 * 作者：GuoJun  完成日期：2022/04/01
 */
#pragma once
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <atomic>
#include <future>             // std::future 获取线程数据
#include <condition_variable> // 条件量，用于线程间通信，唤醒阻塞线程
#include <thread>             // std::thread 线程相关
#include <mutex>              // std::mutex, std::unique_lock 互斥量
#include <functional>         // std::function 函数对象
#include <stdexcept>          // std::runtime_error   标准异常

namespace std
{
//线程池最大容量,应尽量设小一点
#define THREADPOOL_MAX_NUM 16
#define THREADPOOL_AUTO_GROW //动态增加线程池容量

    //线程池,可以提交变参函数或拉姆达表达式的匿名函数执行,可以获取执行返回值
    //不直接支持类成员函数, 支持类静态成员函数或全局函数,opertor()函数等
    class ThreadPool
    {
        vector<thread> _workers;        //工作线程
        queue<function<void()>> _tasks; //任务队列
        mutex _queue_mutex;             //互斥量
        condition_variable _condition;  //条件阻塞
        atomic<bool> _run{true};        //线程池是否执行
        atomic<int> _idlThrNum{0};      //空闲线程数量

    public:
        inline ThreadPool(size_t size) { addThread(size); }
        inline ~ThreadPool()
        {
            {
                unique_lock<mutex> lock(_queue_mutex);
                _run = false;
            }
            _condition.notify_all(); // 唤醒所有线程执行
            for (thread &worker : _workers)
            {
                // worker.detach(); // 让线程“自生自灭”
                if (worker.joinable())
                    worker.join(); // 等待任务结束， 前提：线程一定会执行完
            }
        }

    public:
        // 提交一个任务
        // 调用.get()获取返回值会等待任务执行完,获取返回值
        template <class F, class... Args>
        auto enqueue(F &&f, Args &&...args) -> future<decltype(f(args...))>
        {

            using return_type = decltype(f(args...));
            // typename std::result_of<F(Args...)>::type, 函数 f 的返回值类型
            auto task = make_shared<packaged_task<return_type()>>(bind(forward<F>(f), forward<Args>(args)...)); // 把函数入口及参数,打包(绑定)

            future<return_type> res = task->get_future(); // 获取任务执行结果

            {                                         // 添加任务到队列
                lock_guard<mutex> lock{_queue_mutex}; //对当前块的语句加锁  lock_guard 是 mutex 的 stack 封装类，构造的时候 lock()，析构的时候 unlock()

                if (!_run) // stoped
                    throw runtime_error("ThreadPool is stopped.");

                _tasks.emplace([task]()
                               { (*task)(); }); // push(Task{...}) 放到队列后面
            }

#ifdef THREADPOOL_AUTO_GROW
            if (_idlThrNum < 1 && _workers.size() < THREADPOOL_MAX_NUM)
                addThread(1);
#endif

            _condition.notify_one(); // 通知工作线程,唤醒一个线程执行
            return res;
        }

        //空闲线程数量
        int idlCount() { return _idlThrNum; }
        //线程数量
        int thrCount() { return _workers.size(); }

#ifndef THREADPOOL_AUTO_GROW

    private:
#endif // !THREADPOOL_AUTO_GROW
       //添加指定数量的线程
        void addThread(unsigned short size)
        {
            for (; _workers.size() < THREADPOOL_MAX_NUM && size > 0; --size) //增加线程数量,但不超过 预定义数量 THREADPOOL_MAX_NUM
            {
                //工作线程函数
                _workers.emplace_back(
                    [this]
                    {
                        while (_run)
                        {
                            function<void()> task; // 获取一个待执行的任务对象
                            {
                                unique_lock<mutex> lock{_queue_mutex}; // unique_lock 相比 lock_guard 的好处是：可以随时 unlock() 和 lock()
                                _condition.wait(lock,
                                                [this]
                                                { return !_run || !_tasks.empty(); }); // 等待条件量，等待任务队列不为空（condition是返回false才会wait）
                                                                                       //如果线程池终止，且任务队列为空
                                if (!_run && _tasks.empty())
                                    return;
                                task = move(_tasks.front()); // 按先进先出从队列取一个任务
                                _tasks.pop();
                            }
                            _idlThrNum--;
                            task(); //执行任务
                            _idlThrNum++;
                        }
                    });
                _idlThrNum++;
            }
        }
    };

}

#endif
