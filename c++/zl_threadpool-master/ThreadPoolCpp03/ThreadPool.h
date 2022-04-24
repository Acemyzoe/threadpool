#pragma once  
#include <deque>  
#include <pthread.h>
#include <functional>      // for std::function, std::bind

// ʹ��C++03/C++0x ���Թ淶ʵ�ֵ��̳߳أ� ���ڶ���������ÿһ��job����һ��function����
namespace zl
{
    class ThreadPool
    {
    public:
        typedef std::function<void()> Task;

    public:
        ThreadPool(int threadNum = 10);
        ~ThreadPool();

    public:
        size_t addTask(const Task& task);
        void   stop();
        int    size();
        Task take();

    private:
        int createThreads();
        static void* threadFunc(void * threadData);

    private:
        ThreadPool& operator=(const ThreadPool&);
        ThreadPool(const ThreadPool&);

    private:
        volatile  bool              isRunning_;
        int                         threadsNum_;
        pthread_t*                  threads_;

        std::deque<Task>            taskQueue_;
        pthread_mutex_t             mutex_;
        pthread_cond_t              condition_;
    };
}  
