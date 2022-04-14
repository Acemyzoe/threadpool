#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <functional>
#include <vector>
#include <thread>

#include "CMessageQueue.h"

#define MIN_THREADS 1

template <class Type>
class CThreadPool
{
    CThreadPool &operator=(const CThreadPool &) = delete;
    CThreadPool(const CThreadPool &other) = delete;

public:
    CThreadPool(int32_t threads,
                std::function<void(Type &record, CThreadPool<Type> *pSub)> handler);
    virtual ~CThreadPool();

    void Run();
    virtual void PreHandler() {}
    virtual void AfterHandler() {}
    void Submit(Type record);

private:
    bool _shutdown;
    int32_t _threads;
    std::function<void(Type &record, CThreadPool<Type> *pSub)> _handler;
    std::vector<std::thread> _workers;
    CMessageQueue<Type> _tasks;
};

template <class Type>
CThreadPool<Type>::CThreadPool(int32_t threads,
                               std::function<void(Type &record, CThreadPool<Type> *pSub)> handler)
    : _shutdown(false),
      _threads(threads),
      _handler(handler),
      _workers(),
      _tasks()
{
}

template <class Type>
void CThreadPool<Type>::Run()
{
    if (_threads < MIN_THREADS)
        _threads = MIN_THREADS;
    for (int32_t i = 0; i < _threads; i++)
    {
        _workers.emplace_back(
            [this]
            {
                PreHandler();
                while (!_shutdown)
                {
                    Type record;
                    _tasks.Pop(record, true);
                    _handler(record, this);
                }
                AfterHandler();
            });
    }
}

template <class Type>
CThreadPool<Type>::~CThreadPool()
{
    for (std::thread &worker : _workers)
        worker.join();
}

template <class Type>
void CThreadPool<Type>::Submit(Type record)
{
    _tasks.Push(record);
}
#endif // !THREAD_POOL_H