#include <execq/execq.h>
#include <iostream>
// The function is called in parallel on the next free thread
// with the next object from the queue.
size_t GetStringSize(const std::atomic_bool &isCanceled, std::string &&object)
{
    if (isCanceled)
    {
        std::cout << "Queue has been canceled. Skipping object...";
        return 0;
    }

    std::cout << "Processing object: " << object << '\n';

    return object.size();
}

int main(void)
{
    std::shared_ptr<execq::IExecutionPool> pool = execq::CreateExecutionPool();

    std::unique_ptr<execq::IExecutionQueue<size_t(std::string)>> queue = execq::CreateConcurrentExecutionQueue<size_t, std::string>(pool, &GetStringSize);

    std::future<size_t> future1 = queue->push("qwe");
    std::future<size_t> future2 = queue->push("some string");
    std::future<size_t> future3 = queue->push("hello future");

    const size_t totalSize = future1.get() + future2.get() + future3.get();

    return 0;
}