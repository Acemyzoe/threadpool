#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/asio.hpp>

class ThreadPool
{
public:
    explicit ThreadPool(std::size_t size)
        : work_guard_(boost::asio::make_work_guard(io_context_))
    {
        workers_.reserve(size);
        for (std::size_t i = 0; i < size; ++i)
        {
            workers_.emplace_back(&boost::asio::io_context::run, &io_context_);
        }
    }

    ~ThreadPool()
    {
        io_context_.stop();

        for (auto &w : workers_)
        {
            w.join();
        }
    }

    // Add new work item to the pool.
    template <class F>
    void enqueue(F f)
    {
        boost::asio::post(io_context_, f);
    }

private:
    std::vector<std::thread> workers_;
    boost::asio::io_context io_context_;

    typedef boost::asio::io_context::executor_type ExecutorType;
    boost::asio::executor_work_guard<ExecutorType> work_guard_;
};